# Tarsau - Basit Arşivleme Aracı

Tarsau, Linux sistemler için geliştirilmiş, metin (ASCII) dosyalarını sıkıştırma yapmadan tek bir dosya altında birleştiren (arşivleyen) ve daha sonra bu arşivden orijinal halleriyle geri çıkartan C tabanlı bir komut satırı aracıdır. `tar` komutunun basitleştirilmiş bir türevi olarak tasarlanmıştır.

Sistem Programlama Dersi 2025-2026 Bahar Dönemi Proje Ödevi kapsamında geliştirilmiştir.

## Özellikler

- **Arşivleme (`-b`)**: Birden fazla ASCII metin dosyasını `.sau` uzantılı tek bir dosyada birleştirir.
- **Çıkartma (`-a`)**: `.sau` uzantılı arşiv dosyalarını mevcut dizine veya belirtilen yeni bir dizine orijinal içerikleri ve okuma/yazma/çalıştırma izinlerini (rwx) koruyarak çıkartır.
- **Dosya Doğrulama**: Arşivlenecek dosyaların sadece ASCII formatında olmasını zorunlu kılar.
- **Kapasite Limitleri**: Tek bir arşiv işleminde en fazla 32 dosya kabul edilir ve dosyaların toplam boyutu 200 MB'ı aşamaz.

## Derleme ve Kurulum

Proje Linux (veya WSL) ortamında çalıştırılmak üzere tasarlanmıştır ve POSIX kütüphanelerini kullanır. 

Derlemek için proje dizininde terminali açıp şu komutu çalıştırmanız yeterlidir:

```bash
make
```

Bu komut, `tarsau` isimli çalıştırılabilir dosyayı üretecektir. Eski derlemeleri temizlemek için `make clean` komutunu kullanabilirsiniz.

## Kullanım

### Arşivleme Yapmak

Birden fazla metin dosyasını birleştirmek için `-b` parametresi kullanılır.

```bash
# Varsayılan (a.sau) isimle arşivleme:
./tarsau -b dosya1.txt dosya2.txt dosya3.txt

# Özel isim belirterek arşivleme:
./tarsau -b dosya1.txt dosya2.txt -o benim_arsivim.sau
```

### Arşivden Çıkartmak

Bir arşivi çıkartmak için `-a` parametresi kullanılır.

```bash
# Arşivi mevcut (bulunduğunuz) dizine çıkartmak:
./tarsau -a benim_arsivim.sau

# Arşivi yeni oluşturulacak (veya mevcut) bir dizine çıkartmak:
./tarsau -a benim_arsivim.sau cikartilacak_klasor
```

## Arşiv Formatı Yapısı (.sau)

`.sau` uzantılı dosyalar iki temel bölümden oluşur:
1. **Organizasyon (İçerik) Bölümü:** Dosyanın ilk 10 baytı bu bölümün boyutunu belirtir. Daha sonra her bir dosya için `|Dosya adı,İzinler,Boyut|` formatında meta veriler saklanır.
2. **Arşiv Verisi Bölümü:** Organizasyon bölümünden hemen sonra hiçbir ayrıştırıcı olmadan dosyaların ham ASCII verileri peş peşe yazılır.

## Lisans ve Kısıtlamalar
Bu proje akademik amaçlı geliştirilmiştir. Sadece Linux tabanlı işletim sistemleri üzerinde (stat, chmod, mkdir gibi POSIX standartları kullanıldığı için) tam verimli çalışır.
