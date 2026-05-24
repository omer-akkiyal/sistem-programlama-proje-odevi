#ifndef TARSAU_H
#define TARSAU_H

// Proje kısıtlamaları
#define MAX_FILES 32                      // Giriş dosyası sayısı en fazla 32 olabilir
#define MAX_TOTAL_SIZE (200 * 1024 * 1024) // Toplam boyut 200 MB'ı geçemez

// Fonksiyon prototipleri
int is_ascii(const char *filename);
long get_file_size(const char *filename);
int get_file_permissions(const char *filename);
void archive_files(int file_count, char *files[], const char *output_file);
void extract_archive(const char *archive_file, const char *target_dir);

#endif