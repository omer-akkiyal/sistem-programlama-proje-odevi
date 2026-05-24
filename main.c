#include <stdio.h>
#include <string.h>
#include "tarsau.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Kullanim: tarsau -b dosya1 dosya2 ... [-o cikti.sau] VEYA tarsau -a arsiv.sau [dizin]\n");
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        /* ----- ARŞİVLEME MODU (-b) ----- */
        char *input_files[MAX_FILES];
        int file_count = 0;
        char *output_file = "a.sau"; // Varsayılan çıktı dosyası adı
        long total_size = 0;

        // Komut satırı argümanlarını ayrıştırma
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    output_file = argv[i + 1]; // -o parametresinden sonra belirtilen ad
                    break; 
                }
            } else {
                if (file_count >= MAX_FILES) {
                    printf("Hata: Giris dosyasi sayisi en fazla 32 olabilir.\n");
                    return 1;
                }
                input_files[file_count++] = argv[i];
            }
        }

        // Giriş dosyalarının kurallara uygunluğunu denetleme
        for (int i = 0; i < file_count; i++) {
            // ASCII (metin) dosyası kontrolü
            if (!is_ascii(input_files[i])) {
                printf("%s giris dosyasinin formati uyumsuzdur!\n", input_files[i]);
                return 1;
            }
            
            // Dosya boyutunu al ve toplam boyutu hesapla
            long size = get_file_size(input_files[i]);
            if (size == -1) {
                printf("Hata: %s dosyasi okunamadi!\n", input_files[i]);
                return 1;
            }
            total_size += size;
        }

        // Toplam boyut sınırını kontrol et (200 MB)
        if (total_size > MAX_TOTAL_SIZE) {
            printf("Hata: Giris dosyalarinin toplam boyutu 200 MB'i gecemez.\n");
            return 1;
        }

        // Tüm kontroller geçildi, arşivleme işlemini başlat
        archive_files(file_count, input_files, output_file);
    } 
    else if (strcmp(argv[1], "-a") == 0) {
        /* ----- ÇIKARMA MODU (-a) ----- */
        if (argc < 3 || argc > 4) {
            printf("Kullanim: tarsau -a arsiv.sau [dizin]\n");
            return 1;
        }

        char *archive_file = argv[2];
        char *target_dir = NULL;

        // Hedef dizin parametresi varsa ayarla
        if (argc == 4) {
            target_dir = argv[3];
        }

        // Arşivden çıkarma fonksiyonunu çağır
        extract_archive(archive_file, target_dir);
    } 
    else {
        printf("Gecersiz parametre.\n");
    }

    return 0;
}