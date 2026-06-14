# OpenMPI-Kelompok2

[cite_start]**Tugas Kelompok: Komputasi Paralel Berbasis Cluster dengan OpenMPI** [cite: 1]
[cite_start]**Mata Kuliah:** Komputasi Paralel / Sistem Terdistribusi (UIN Sunan Kalijaga) [cite: 2]
**Dosen Pengampu:** Bapak Adi Wirawan

## Anggota Kelompok
[cite_start]Proyek ini dikerjakan oleh tim yang terdiri dari maksimal 5 orang untuk mensimulasikan arsitektur *Cluster/Multi-node*[cite: 3, 21]:

1. **24106050005** NURUL FATIHA 
2. **24106050006** GAHYAKA ARARYA FAIRUZ
3. **24106050007** MUHAMMAD FAJRI 
4. **24106050010** HESTI FEBRIYANI 
5. **24106050011** FARIZ HUSAIN ALBAR
6. **24106050017** YOUNATA NUR ROCHMAN

---

## Deskripsi Proyek
[cite_start]Aplikasi komputasi paralel ini dibangun menggunakan bahasa C dengan *library* OpenMPI[cite: 5]. [cite_start]Tujuan utama dari aplikasi ini adalah memproses data akademik berskala besar yang disimpan dalam format `.csv`[cite: 5]. 

[cite_start]Aplikasi yang dibuat mampu membagi beban kerja secara adil ke beberapa *hosts* (Master & Workers) untuk menghitung tiga komponen utama dari data tersebut[cite: 6]:
1. [cite_start]**Nilai Maksimum** [cite: 7]
2. [cite_start]**Nilai Minimum** [cite: 8]
3. [cite_start]**Nilai Rata-rata (Average)** [cite: 9]

---

## Struktur Repositori
Repositori ini berisi file-file esensial yang dibutuhkan untuk menjalankan program:
* [cite_start]`mpi_nilai.c`: *Source code* utama aplikasi OpenMPI[cite: 53].
* [cite_start]`data_nilai.csv`: File *input* data yang terdiri dari 100 baris[cite: 10, 19, 53]. [cite_start]Sesuai spesifikasi, file ini berisi 2 kolom yaitu NIM (String/Integer) dan Nilai (Float/Integer dengan rentang 0-100)[cite: 11, 12, 13].
* [cite_start]`hosts.txt`: File konfigurasi jaringan yang memuat daftar IP dari komputer atau *Container Docker* setiap anggota kelompok[cite: 24, 25, 53].

---

## Skenario Arsitektur (Master & Worker)
[cite_start]Pengeksekusian program mengandalkan komunikasi *multi-node* (didukung melalui lingkungan VPS dan Docker *Container*)[cite: 21]:
* [cite_start]**Master Node (Rank 0):** Bertanggung jawab penuh untuk membaca file `data_nilai.csv`, memotong-motong data menjadi bagian kecil (*Scatter*), mengirimkannya ke seluruh *Worker*, menerima hasil perhitungan kembali (*Gather/Reduce*), dan mencetak hasil akhir ke layar terminal[cite: 22].
* [cite_start]**Worker Nodes (Rank 1 - 4):** Bertugas menerima potongan data dari Master, melakukan perhitungan lokal (mencari *max*, *min*, dan *sum* lokal), kemudian mengirimkannya kembali ke Master Node[cite: 23].

---

## Cara Eksekusi Program
[cite_start]Pastikan Anda sudah mengonfigurasi jalur SSH *passwordless* dan mengatur *path* direktori yang sama antar *node* (sesuai dokumentasi Laporan Konfigurasi Lingkungan)[cite: 50, 51].

**1. Kompilasi Program**
```bash
mpicc -o mpi_nilai mpi_nilai.c
```

**2. Eksekusi Cluster**
Program **wajib** dieksekusi menggunakan perintah `mpirun` dengan menentukan argumen `-np 5` dan memanfaatkan file `hosts.txt`.

```bash
mpirun --hostfile hosts.txt -np 5 ./mpi_nilai
```

---

## 📊 Format Output
Setelah program selesai dieksekusi, Rank 0 (Master) akan menampilkan *output* di terminal dengan format berikut:

```text
===========================================
 HASIL ANALISIS NILAI MAHASISWA (MPI)
===========================================
Total Data Diproses : [Jumlah Baris] baris
Nilai Tertinggi     : [Angka]
Nilai Terendah      : [Angka]
Rata-rata Kelas     : [Angka]
===========================================
```
