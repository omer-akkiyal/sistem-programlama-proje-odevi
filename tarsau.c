#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tarsau.h"

/*
 * Dosyanın yalnızca metin (ASCII, karakter başına 1 bayt) içerip
 * içermediğini kontrol eder. ASCII dışı karakter bulunursa 0 döner.
 */
int is_ascii(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c < 0 || c > 127) {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;
}

/* stat() sistem çağrısı ile dosya boyutunu bayt cinsinden döndürür. */
long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/* stat() sistem çağrısı ile dosyanın rwx izin bitlerini döndürür. */
int get_file_permissions(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_mode & 0777;
    }
    return 0;
}

/*
 * Arşivleme fonksiyonu: Verilen dosyaları .sau formatında birleştirir.
 * Arşiv formatı: [10 bayt header][organizasyon bölümü][dosya verileri]
 */
void archive_files(int file_count, char *files[], const char *output_file) {
    FILE *out = fopen(output_file, "w");
    if (!out) {
        printf("Hata: Cikti dosyasi (%s) olusturulamadi!\n", output_file);
        return;
    }

    // 1. Organizasyon bölümünün metin dizisini oluştur
    char org_buffer[8192] = "";
    
    for (int i = 0; i < file_count; i++) {
        long size = get_file_size(files[i]);
        int perms = get_file_permissions(files[i]);
        
        char temp_info[256];
        // Her kayıt: |dosyaadi,izin(octal),boyut biçiminde eklenir
        sprintf(temp_info, "|%s,%o,%ld", files[i], perms, size);
        strcat(org_buffer, temp_info);
    }
    // Organizasyon bölümünü kapatan son ayırıcı
    strcat(org_buffer, "|");
    
    long org_size = strlen(org_buffer);

    // 2. İlk 10 bayta organizasyon boyutunu yaz (sıfır dolgulu, örn: "0000000142")
    fprintf(out, "%010ld", org_size);

    // 3. Organizasyon verisini dosyaya yaz
    fprintf(out, "%s", org_buffer);

    // 4. Dosya içeriklerini sırayla oku ve arşive ekle
    for (int i = 0; i < file_count; i++) {
        FILE *in = fopen(files[i], "r");
        if (in) {
            int c;
            while ((c = fgetc(in)) != EOF) {
                fputc(c, out);
            }
            fclose(in);
        } else {
            printf("Uyari: %s dosyasi okunurken atlandi!\n", files[i]);
        }
    }

    fclose(out);
    printf("Dosyalar birlestirildi.\n");
}

/*
 * Çıkarma fonksiyonu: .sau arşiv dosyasından dosyaları geri çıkarır.
 * Organizasyon bölümünü okuyarak dosya adı, izin ve boyut bilgilerini elde eder,
 * ardından her dosyayı orijinal izinleriyle birlikte oluşturur.
 */
void extract_archive(const char *archive_file, const char *target_dir) {
    FILE *in = fopen(archive_file, "r");
    if (!in) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        return;
    }

    // 1. Header: İlk 10 baytı oku (organizasyon bölümü boyutu)
    char header[11] = {0};
    if (fread(header, 1, 10, in) != 10) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        return;
    }

    long org_size = atol(header);
    if (org_size <= 0) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        return;
    }

    // 2. Organizasyon bölümünü oku
    char *org_data = malloc(org_size + 1);
    if (!org_data) {
        printf("Hata: Bellek tahsisi basarisiz!\n");
        fclose(in);
        return;
    }
    if ((long)fread(org_data, 1, org_size, in) != org_size) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        free(org_data);
        fclose(in);
        return;
    }
    org_data[org_size] = '\0';

    // 3. Hedef dizin belirtilmişse ve yoksa oluştur
    if (target_dir != NULL) {
        struct stat st = {0};
        if (stat(target_dir, &st) == -1) {
            mkdir(target_dir, 0755);
        }
    }

    // 4. Organizasyon verisini parçala ve dosyaları oluştur
    long data_offset = 10 + org_size;
    long current_file_pos = data_offset;

    // '|' ayırıcısına göre metni parçalara ayır
    char *token = strtok(org_data, "|");
    
    while (token != NULL) {
        // Boş token kontrolü
        if (strlen(token) == 0) {
            token = strtok(NULL, "|");
            continue;
        }

        char filename[256];
        int perms;
        long size;
        
        // Kayıt formatı: "dosyaadi,izin,boyut" verisini değişkenlere aktar
        if (sscanf(token, "%255[^,],%o,%ld", filename, &perms, &size) != 3) {
            token = strtok(NULL, "|");
            continue;
        }

        // Çıktı yolunu oluştur
        char out_path[512];
        if (target_dir != NULL) {
            sprintf(out_path, "%s/%s", target_dir, filename);
        } else {
            strcpy(out_path, filename);
        }

        // Dosya verisinin başladığı konuma git
        fseek(in, current_file_pos, SEEK_SET);

        // Dosyayı oluştur ve içeriğini yaz
        FILE *out = fopen(out_path, "w");
        if (out) {
            long bytes_left = size;
            int c;
            while (bytes_left > 0 && (c = fgetc(in)) != EOF) {
                fputc(c, out);
                bytes_left--;
            }
            fclose(out);
            
            // Dosyanın orijinal izinlerini geri yükle
            chmod(out_path, perms);
        } else {
            printf("Uyari: %s dosyasi olusturulamadi!\n", out_path);
        }
        
        // Sonraki dosyanın veri başlangıç konumunu güncelle
        current_file_pos += size;
        
        // Sonraki kayda geç
        token = strtok(NULL, "|");
    }

    free(org_data);
    fclose(in);

    if (target_dir) {
        printf("%s dizininde dosyalar acildi.\n", target_dir);
    } else {
        printf("Dosyalar bulundugunuz dizinde acildi.\n");
    }
}