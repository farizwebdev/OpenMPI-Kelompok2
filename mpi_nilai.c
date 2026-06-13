/*
 * mpi_nilai.c
 * Komputasi Paralel - Analisis Nilai Mahasiswa menggunakan OpenMPI
 * Tugas Kelompok - Sistem Terdistribusi
 *
 * Cara kompilasi:
 *   mpicc -o mpi_nilai mpi_nilai.c
 *
 * Cara eksekusi (lokal, tanpa hostfile):
 *   mpirun -np 5 ./mpi_nilai
 *
 * Cara eksekusi (multi-node dengan hostfile):
 *   mpirun --hostfile hosts.txt -np 5 ./mpi_nilai
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define MAX_DATA   200   /* Kapasitas maksimum baris CSV */
#define MAX_LINE   128   /* Panjang maksimum satu baris */
#define CSV_FILE   "data_nilai.csv"

/* ─── Fungsi Bantu: Baca CSV ─────────────────────────────────────────── */
int baca_csv(const char *filename, double *nilai, int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] Tidak bisa membuka file: %s\n", filename);
        return -1;
    }

    char line[MAX_LINE];
    int count = 0;

    /* Lewati header */
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && count < max) {
        char *comma = strchr(line, ',');
        if (comma) {
            nilai[count++] = atof(comma + 1);
        }
    }
    fclose(fp);
    return count;
}

/* ═══════════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* ── Variabel global (dipakai Master) ─────────────────────────────── */
    double  all_nilai[MAX_DATA];
    int     total_data = 0;

    /* ── Variabel distribusi ──────────────────────────────────────────── */
    int sendcounts[size], displs[size];   /* berapa elemen tiap proses */
    int base, rem, offset;

    /* ================================================================== */
    /* RANK 0 – MASTER                                                     */
    /* ================================================================== */
    if (rank == 0) {
        /* 1. Baca CSV */
        total_data = baca_csv(CSV_FILE, all_nilai, MAX_DATA);
        if (total_data < 0) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("[Master] File '%s' berhasil dibaca: %d baris.\n",
               CSV_FILE, total_data);

        /* 2. Hitung pembagian data ke setiap proses */
        base = total_data / size;   /* porsi dasar */
        rem  = total_data % size;   /* sisa yang dibagi ke proses pertama */
        offset = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = base + (i < rem ? 1 : 0);
            displs[i]     = offset;
            offset       += sendcounts[i];
        }
    }

    /* ── Broadcast jumlah total data ke semua proses ─────────────────── */
    MPI_Bcast(&total_data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* ── Broadcast sendcounts & displs ───────────────────────────────── */
    MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs,     size, MPI_INT, 0, MPI_COMM_WORLD);

    /* ── Alokasi buffer lokal ─────────────────────────────────────────── */
    int local_n = sendcounts[rank];
    double *local_nilai = (double *)malloc(local_n * sizeof(double));
    if (!local_nilai) {
        fprintf(stderr, "[Rank %d] Gagal alokasi memori!\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* ================================================================== */
    /* SCATTER – Master mengirim potongan data ke setiap Worker           */
    /* ================================================================== */
    MPI_Scatterv(all_nilai, sendcounts, displs, MPI_DOUBLE,
                 local_nilai, local_n,          MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    printf("[Rank %d] Menerima %d elemen. Contoh[0]=%.1f\n",
           rank, local_n, local_nilai[0]);

    /* ================================================================== */
    /* KOMPUTASI LOKAL – setiap proses menghitung max, min, sum sendiri   */
    /* ================================================================== */
    double local_max = -DBL_MAX;
    double local_min =  DBL_MAX;
    double local_sum = 0.0;

    for (int i = 0; i < local_n; i++) {
        if (local_nilai[i] > local_max) local_max = local_nilai[i];
        if (local_nilai[i] < local_min) local_min = local_nilai[i];
        local_sum += local_nilai[i];
    }

    /* ================================================================== */
    /* REDUCE – Kumpulkan hasil dari semua proses ke Master               */
    /* ================================================================== */
    double global_max, global_min, global_sum;

    MPI_Reduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_min, &global_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    /* ================================================================== */
    /* OUTPUT – Hanya Master (Rank 0) yang mencetak hasil akhir           */
    /* ================================================================== */
    if (rank == 0) {
        double global_avg = global_sum / total_data;

        printf("\n===========================================\n");
        printf("  HASIL ANALISIS NILAI MAHASISWA (MPI)\n");
        printf("===========================================\n");
        printf("Total Data Diproses : %d baris\n", total_data);
        printf("Nilai Tertinggi     : %.1f\n",     global_max);
        printf("Nilai Terendah      : %.1f\n",     global_min);
        printf("Rata-rata Kelas     : %.2f\n",     global_avg);
        printf("===========================================\n");
    }

    free(local_nilai);
    MPI_Finalize();
    return 0;
}
