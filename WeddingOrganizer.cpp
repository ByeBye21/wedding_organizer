/****************************************************************************
**					SAKARYA ÜNİVERSİTESİ
**			BİLGİSAYAR VE BİLİŞİM BİLİMLERİ FAKÜLTESİ
**			    BİLGİSAYAR MÜHENDİSLİĞİ BÖLÜMÜ
**				  PROGRAMLAMAYA GİRİŞİ DERSİ
**	
**				ÖDEV NUMARASI......: Proje Ödevi
**				ÖĞRENCİ ADI........: YOUNES RAHEBI
**				ÖĞRENCİ NUMARASI...: B221210588
**				DERS GRUBU.........: 1. Öğretim B
****************************************************************************/


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

using namespace std;

/*
  Dugunler.txt  (14 alan)
  DugunNo|Tarih|Saat|DamatAdi|DamatBabaAdi|GelinAdi|GelinAnneAdi|PatronAdi|PatronTel|DamatTel|SalonNo|KisiSayisi|SalonUcreti|ToplamUcret

  Davetliler.txt (8 alan)
  DugunNo|DavetliNo|Ad|Soyad|Taraf(D/G)|Adres|Telefon|Durum(E/H)

  Masraf.txt (5 alan)
  DugunNo|Kalem|Kategori|Adet|BirimFiyat

  Masalar.txt (7 alan)
  DugunNo|Kapasite|MasaNo|Taraf(D/G)|DavetliNo|Ad|Soyad
*/

const string DUGUN_DOSYA   = "Dugunler.txt";
const string DAVETLI_DOSYA = "Davetliler.txt";
const string MASRAF_DOSYA  = "Masraf.txt";
const string MASA_RAPOR    = "Masalar.txt";



// ======================== Veri Yapıları ========================

// Düğün kaydı
struct Dugun {
    int dugunNo;
    string tarih;   // YYYY-MM-DD
    string saat;    // HH:MM
    string damatAdi;
    string damatBabaAdi;
    string gelinAdi;
    string gelinAnneAdi;
    string patronAdi;
    string patronTel;
    string damatTel;
    int salonNo;
    int kisiSayisi;
    double salonUcreti;
    double toplamUcret;
};

// Davetli kaydı
struct Davetli {
    int dugunNo;
    int davetliNo;
    string ad;
    string soyad;
    char taraf;     // D (Damat) veya G (Gelin)
    string adres;
    string tel;
    char durum;     // E (gelecek) veya H (gelemeyecek)
};

// Masraf kaydı
struct Masraf {
    int dugunNo;
    string kalem;
    string kategori;
    int adet;
    double birimFiyat;
};

// Masa kaydı (Masalar.txt için)
struct MasaKaydi {
    int dugunNo;
    int kapasite;
    int masaNo;
    char taraf;       // D veya G
    int davetliNo;
    string ad;
    string soyad;
};

// Parse sonuçları
struct IntSonuc { int ok; int deger; };
struct DblSonuc { int ok; double deger; };
struct Alanlar  { int n; string a[30]; };

// Satır -> struct sonuçları
struct DugunOkuSonuc   { int ok; Dugun d; };
struct DavetliOkuSonuc { int ok; Davetli v; };
struct MasrafOkuSonuc  { int ok; Masraf m; };
struct MasaOkuSonuc    { int ok; MasaKaydi k; };

struct DugunBulS { int bulundu; Dugun d; };



// ======================== Ekran / Bekleme ========================

// Ekranı temizle (Windows: cls, diğerleri: clear)
void EkraniTemizle() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Kullanıcı görsün diye ENTER beklet
void DevamIcinEnter() {
    cout << "\nDevam etmek icin ENTER tusuna basin...";
    string dummy;
    getline(cin, dummy);
}

// Her işlem sonunda: ENTER bekle + ekran temizle
void IslemSonuTemizle() {
    DevamIcinEnter();
    EkraniTemizle();
}



// ======================== String yardımcıları ========================

// Baştaki/sondaki boşlukları kırpar (iç boşluklara dokunmaz)
string Trim(string s) {
    int bas = 0;
    while (bas < (int)s.size() && (s[bas] == ' ' || s[bas] == '\t' || s[bas] == '\r' || s[bas] == '\n')) bas++;

    int son = (int)s.size() - 1;
    while (son >= bas && (s[son] == ' ' || s[son] == '\t' || s[son] == '\r' || s[son] == '\n')) son--;

    string r = "";
    for (int i = bas; i <= son; i++) r += s[i];
    return r;
}

// Dosya ayıracı '|' olduğu için kullanıcı girişinde '|' yasak
int IcindePipeVarMi(string s) {
    for (int i = 0; i < (int)s.size(); i++) if (s[i] == '|') return 1;
    return 0;
}

// Küçük harfi büyüğe çevirir (D/G, E/H)
char BuyukHarf(char c) {
    if (c >= 'a' && c <= 'z') return char(c - 'a' + 'A');
    return c;
}

// Rakam kontrolü
int RakamMi(char c) { return (c >= '0' && c <= '9') ? 1 : 0; }



// ======================== Güvenli kullanıcı girişi ========================

// Satır alır. bosOlabilir=0 ise boş giriş kabul edilmez.
string SatirAl(string mesaj, int bosOlabilir) {
    while (true) {
        cout << mesaj;
        string s;
        getline(cin, s);
        s = Trim(s);

        if (!bosOlabilir && s == "") {
            cout << "Bos birakilamaz. Tekrar girin.\n";
            continue;
        }
        if (IcindePipeVarMi(s)) {
            cout << "Girdide '|' karakteri olamaz. Tekrar girin.\n";
            continue;
        }
        return s;
    }
}

// string -> int (hata kontrolü ile)
IntSonuc IntParse(string s) {
    IntSonuc r; r.ok = 0; r.deger = 0;
    s = Trim(s);
    if (s == "") return r;

    stringstream ss(s);
    int x; char fazlalik;
    if (!(ss >> x)) return r;       // sayı değil
    if (ss >> fazlalik) return r;   // fazlalık var
    r.ok = 1; r.deger = x;
    return r;
}

// string -> double (12,5 -> 12.5)
DblSonuc DoubleParse(string s) {
    DblSonuc r; r.ok = 0; r.deger = 0.0;
    s = Trim(s);
    if (s == "") return r;

    for (int i = 0; i < (int)s.size(); i++) if (s[i] == ',') s[i] = '.';

    stringstream ss(s);
    double x; char fazlalik;
    if (!(ss >> x)) return r;
    if (ss >> fazlalik) return r;
    r.ok = 1; r.deger = x;
    return r;
}

// Güvenli int alır (aralık kontrolü ile)
int IntAl(string mesaj, int minDeger, int maxDeger) {
    while (true) {
        string s = SatirAl(mesaj, 0);
        IntSonuc p = IntParse(s);
        if (!p.ok) { cout << "Gecersiz sayi. Tekrar girin.\n"; continue; }
        if (p.deger < minDeger || p.deger > maxDeger) {
            cout << "Aralik disi. (" << minDeger << "-" << maxDeger << ") Tekrar girin.\n";
            continue;
        }
        return p.deger;
    }
}

// Güvenli double alır (min kontrolü ile)
double DoubleAl(string mesaj, double minDeger) {
    while (true) {
        string s = SatirAl(mesaj, 0);
        DblSonuc p = DoubleParse(s);
        if (!p.ok) { cout << "Gecersiz sayi. Tekrar girin.\n"; continue; }
        if (p.deger < minDeger) {
            cout << "Minimum " << minDeger << " olmali. Tekrar girin.\n";
            continue;
        }
        return p.deger;
    }
}

// İki seçenekten birini alır (D/G veya E/H)
char CharSecimAl(string mesaj, char a, char b) {
    a = BuyukHarf(a);
    b = BuyukHarf(b);
    while (true) {
        string s = SatirAl(mesaj, 0);
        if ((int)s.size() != 1) { cout << "Tek karakter girin.\n"; continue; }
        char c = BuyukHarf(s[0]);
        if (c != a && c != b) { cout << "Sadece '" << a << "' veya '" << b << "' girin.\n"; continue; }
        return c;
    }
}



// ======================== Tarih / Saat / Telefon doğrulama ========================

int ArtikYilMi(int yil) {
    if (yil % 400 == 0) return 1;
    if (yil % 100 == 0) return 0;
    if (yil % 4 == 0) return 1;
    return 0;
}

// YYYY-MM-DD kontrol
int TarihGecerliMi(string s) {
    if ((int)s.size() != 10) return 0;
    if (s[4] != '-' || s[7] != '-') return 0;

    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!RakamMi(s[i])) return 0;
    }

    int yil = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
    int ay  = (s[5]-'0')*10 + (s[6]-'0');
    int gun = (s[8]-'0')*10 + (s[9]-'0');

    if (yil < 1900 || yil > 2100) return 0;
    if (ay < 1 || ay > 12) return 0;

    int gunSay = 31;
    if (ay == 4 || ay == 6 || ay == 9 || ay == 11) gunSay = 30;
    if (ay == 2) gunSay = ArtikYilMi(yil) ? 29 : 28;

    if (gun < 1 || gun > gunSay) return 0;
    return 1;
}

// HH:MM kontrol
int SaatGecerliMi(string s) {
    if ((int)s.size() != 5) return 0;
    if (s[2] != ':') return 0;
    if (!RakamMi(s[0]) || !RakamMi(s[1]) || !RakamMi(s[3]) || !RakamMi(s[4])) return 0;

    int saat = (s[0]-'0')*10 + (s[1]-'0');
    int dk   = (s[3]-'0')*10 + (s[4]-'0');

    if (saat < 0 || saat > 23) return 0;
    if (dk < 0 || dk > 59) return 0;
    return 1;
}

// Telefon kontrol: + opsiyonel, sonrası rakam, 10-15 hane
int TelGecerliMi(string s) {
    if (s == "") return 0;
    int i = 0;
    if (s[0] == '+') i = 1;

    int rakamSay = 0;
    for (; i < (int)s.size(); i++) {
        if (!RakamMi(s[i])) return 0;
        rakamSay++;
    }
    if (rakamSay < 10 || rakamSay > 15) return 0;
    return 1;
}

string TarihAl(string mesaj) {
    while (true) {
        string s = SatirAl(mesaj, 0);
        if (!TarihGecerliMi(s)) { cout << "Tarih hatali. Ornek: 2025-12-20\n"; continue; }
        return s;
    }
}

string SaatAl(string mesaj) {
    while (true) {
        string s = SatirAl(mesaj, 0);
        if (!SaatGecerliMi(s)) { cout << "Saat hatali. Ornek: 19:30\n"; continue; }
        return s;
    }
}

string TelAl(string mesaj) {
    while (true) {
        string s = SatirAl(mesaj, 0);
        if (!TelGecerliMi(s)) {
            cout << "Telefon hatali. Sadece rakam (opsiyonel +), 10-15 hane.\n";
            continue;
        }
        return s;
    }
}

// Güncellemede boş bırakılabilen versiyonlar
string TarihAlBos(string mesaj) {
    while (true) {
        string s = SatirAl(mesaj, 1);
        if (s == "") return s;
        if (!TarihGecerliMi(s)) { cout << "Tarih hatali. Ornek: 2025-12-20\n"; continue; }
        return s;
    }
}
string SaatAlBos(string mesaj) {
    while (true) {
        string s = SatirAl(mesaj, 1);
        if (s == "") return s;
        if (!SaatGecerliMi(s)) { cout << "Saat hatali. Ornek: 19:30\n"; continue; }
        return s;
    }
}
string TelAlBos(string mesaj) {
    while (true) {
        string s = SatirAl(mesaj, 1);
        if (s == "") return s;
        if (!TelGecerliMi(s)) { cout << "Telefon hatali.\n"; continue; }
        return s;
    }
}



// ======================== Dosya satır parçalama ========================

// Satırı '|' karakterine göre böler
Alanlar Bol(string satir) {
    Alanlar r; r.n = 0;
    string tmp = "";
    for (int i = 0; i < (int)satir.size(); i++) {
        if (satir[i] == '|') {
            if (r.n < 30) r.a[r.n++] = tmp;
            tmp = "";
        } else {
            tmp += satir[i];
        }
    }
    if (r.n < 30) r.a[r.n++] = tmp;
    return r;
}



// ======================== Satır - Struct dönüşümleri ========================

// Dugun satırını oku
DugunOkuSonuc DugunSatirOku(string satir) {
    DugunOkuSonuc r; r.ok = 0;
    satir = Trim(satir);
    if (satir == "") return r;

    Alanlar al = Bol(satir);
    if (al.n != 14) return r;

    IntSonuc dugNo  = IntParse(al.a[0]);
    IntSonuc salon  = IntParse(al.a[10]);
    IntSonuc kisi   = IntParse(al.a[11]);
    DblSonuc salonU = DoubleParse(al.a[12]);
    DblSonuc toplam = DoubleParse(al.a[13]);

    if (!dugNo.ok || !salon.ok || !kisi.ok || !salonU.ok || !toplam.ok) return r;

    Dugun d;
    d.dugunNo = dugNo.deger;
    d.tarih = al.a[1];
    d.saat  = al.a[2];
    d.damatAdi = al.a[3];
    d.damatBabaAdi = al.a[4];
    d.gelinAdi = al.a[5];
    d.gelinAnneAdi = al.a[6];
    d.patronAdi = al.a[7];
    d.patronTel = al.a[8];
    d.damatTel  = al.a[9];
    d.salonNo = salon.deger;
    d.kisiSayisi = kisi.deger;
    d.salonUcreti = salonU.deger;
    d.toplamUcret = toplam.deger;

    r.ok = 1;
    r.d = d;
    return r;
}

// Dugun satırı yap
string DugunSatirYap(Dugun d) {
    string s = "";
    s += to_string(d.dugunNo) + "|";
    s += d.tarih + "|";
    s += d.saat + "|";
    s += d.damatAdi + "|";
    s += d.damatBabaAdi + "|";
    s += d.gelinAdi + "|";
    s += d.gelinAnneAdi + "|";
    s += d.patronAdi + "|";
    s += d.patronTel + "|";
    s += d.damatTel + "|";
    s += to_string(d.salonNo) + "|";
    s += to_string(d.kisiSayisi) + "|";
    s += to_string(d.salonUcreti) + "|";
    s += to_string(d.toplamUcret);
    return s;
}

// Davetli satırını oku
DavetliOkuSonuc DavetliSatirOku(string satir) {
    DavetliOkuSonuc r; r.ok = 0;
    satir = Trim(satir);
    if (satir == "") return r;

    Alanlar al = Bol(satir);
    if (al.n != 8) return r;

    IntSonuc dugNo = IntParse(al.a[0]);
    IntSonuc davNo = IntParse(al.a[1]);
    if (!dugNo.ok || !davNo.ok) return r;

    if ((int)al.a[4].size() != 1 || (int)al.a[7].size() != 1) return r;

    char taraf = BuyukHarf(al.a[4][0]);
    char durum = BuyukHarf(al.a[7][0]);

    if (!(taraf == 'D' || taraf == 'G')) return r;
    if (!(durum == 'E' || durum == 'H')) return r;

    Davetli v;
    v.dugunNo = dugNo.deger;
    v.davetliNo = davNo.deger;
    v.ad = al.a[2];
    v.soyad = al.a[3];
    v.taraf = taraf;
    v.adres = al.a[5];
    v.tel = al.a[6];
    v.durum = durum;

    r.ok = 1;
    r.v = v;
    return r;
}

// Davetli satırı yap
string DavetliSatirYap(Davetli v) {
    string s = "";
    s += to_string(v.dugunNo) + "|";
    s += to_string(v.davetliNo) + "|";
    s += v.ad + "|";
    s += v.soyad + "|";
    s += string(1, v.taraf) + "|";
    s += v.adres + "|";
    s += v.tel + "|";
    s += string(1, v.durum);
    return s;
}

// Masraf satırını oku
MasrafOkuSonuc MasrafSatirOku(string satir) {
    MasrafOkuSonuc r; r.ok = 0;
    satir = Trim(satir);
    if (satir == "") return r;

    Alanlar al = Bol(satir);
    if (al.n != 5) return r;

    IntSonuc dugNo = IntParse(al.a[0]);
    IntSonuc adet  = IntParse(al.a[3]);
    DblSonuc fiyat = DoubleParse(al.a[4]);
    if (!dugNo.ok || !adet.ok || !fiyat.ok) return r;

    Masraf m;
    m.dugunNo = dugNo.deger;
    m.kalem = al.a[1];
    m.kategori = al.a[2];
    m.adet = adet.deger;
    m.birimFiyat = fiyat.deger;

    r.ok = 1;
    r.m = m;
    return r;
}

// Masraf satırı yap
string MasrafSatirYap(Masraf m) {
    string s = "";
    s += to_string(m.dugunNo) + "|";
    s += m.kalem + "|";
    s += m.kategori + "|";
    s += to_string(m.adet) + "|";
    s += to_string(m.birimFiyat);
    return s;
}

// MasaKaydi satırını oku (Masalar.txt)
MasaOkuSonuc MasaSatirOku(string satir) {
    MasaOkuSonuc r; r.ok = 0;
    satir = Trim(satir);
    if (satir == "") return r;

    Alanlar al = Bol(satir);
    if (al.n != 7) return r;

    IntSonuc dugNo = IntParse(al.a[0]);
    IntSonuc kap   = IntParse(al.a[1]);
    IntSonuc masa  = IntParse(al.a[2]);
    if (!dugNo.ok || !kap.ok || !masa.ok) return r;

    if ((int)al.a[3].size() != 1) return r;
    char t = BuyukHarf(al.a[3][0]);
    if (!(t == 'D' || t == 'G')) return r;

    IntSonuc davNo = IntParse(al.a[4]);
    if (!davNo.ok) return r;

    MasaKaydi k;
    k.dugunNo = dugNo.deger;
    k.kapasite = kap.deger;
    k.masaNo = masa.deger;
    k.taraf = t;
    k.davetliNo = davNo.deger;
    k.ad = al.a[5];
    k.soyad = al.a[6];

    r.ok = 1;
    r.k = k;
    return r;
}

// MasaKaydi satırı yap
string MasaSatirYap(MasaKaydi k) {
    string s = "";
    s += to_string(k.dugunNo) + "|";
    s += to_string(k.kapasite) + "|";
    s += to_string(k.masaNo) + "|";
    s += string(1, k.taraf) + "|";
    s += to_string(k.davetliNo) + "|";
    s += k.ad + "|";
    s += k.soyad;
    return s;
}



// ======================== Arama kontrolleri ========================

// Dugun var mı?
int DugunVarMi(int dugunNo) {
    ifstream in(DUGUN_DOSYA);
    if (!in.is_open()) return 0;

    string satir;
    while (getline(in, satir)) {
        DugunOkuSonuc r = DugunSatirOku(satir);
        if (r.ok && r.d.dugunNo == dugunNo) { in.close(); return 1; }
    }
    in.close();
    return 0;
}

// Dugun bul
DugunBulS DugunBul(int dugunNo) {
    DugunBulS s; s.bulundu = 0;
    ifstream in(DUGUN_DOSYA);
    if (!in.is_open()) return s;

    string satir;
    while (getline(in, satir)) {
        DugunOkuSonuc r = DugunSatirOku(satir);
        if (r.ok && r.d.dugunNo == dugunNo) {
            s.bulundu = 1;
            s.d = r.d;
            break;
        }
    }
    in.close();
    return s;
}

// Davetli var mı?
int DavetliVarMi(int dugunNo, int davetliNo) {
    ifstream in(DAVETLI_DOSYA);
    if (!in.is_open()) return 0;

    string satir;
    while (getline(in, satir)) {
        DavetliOkuSonuc r = DavetliSatirOku(satir);
        if (r.ok && r.v.dugunNo == dugunNo && r.v.davetliNo == davetliNo) {
            in.close(); return 1;
        }
    }
    in.close();
    return 0;
}



// ======================== Düğün işlemleri ========================

void DugunEkle() {
    Dugun d;

    d.dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    if (DugunVarMi(d.dugunNo)) { cout << "Bu DugunNo zaten var.\n"; return; }

    d.tarih = TarihAl("Tarih (YYYY-MM-DD): ");
    d.saat  = SaatAl("Saat  (HH:MM): ");

    d.damatAdi     = SatirAl("Damat adi: ", 0);
    d.damatBabaAdi = SatirAl("Damat baba adi: ", 0);

    d.gelinAdi     = SatirAl("Gelin adi: ", 0);
    d.gelinAnneAdi = SatirAl("Gelin anne adi: ", 0);

    d.patronAdi = SatirAl("Patron adi: ", 0);
    d.patronTel = TelAl("Patron tel (+905.. veya 05..): ");
    d.damatTel  = TelAl("Damat tel  (+905.. veya 05..): ");

    d.salonNo    = IntAl("Salon no: ", 1, 999999);
    d.kisiSayisi = IntAl("Kisi sayisi: ", 1, 100000);

    d.salonUcreti = DoubleAl("Salon ucreti: ", 0);
    d.toplamUcret = d.salonUcreti; // masraflar eklenince güncellenecek

    ofstream out(DUGUN_DOSYA, ios::app);
    out << DugunSatirYap(d) << "\n";
    out.close();

    cout << "Dugun eklendi.\n";
}

void DugunListele() {
    ifstream in(DUGUN_DOSYA);
    if (!in.is_open()) { cout << "Dugunler.txt yok.\n"; return; }

    cout << "\n--- DUGUN LISTESI ---\n";
    string satir;
    while (getline(in, satir)) {
        DugunOkuSonuc r = DugunSatirOku(satir);
        if (r.ok) {
            cout << "No: " << r.d.dugunNo
                 << " | " << r.d.tarih << " " << r.d.saat
                 << " | Salon: " << r.d.salonNo
                 << " | Kisi: " << r.d.kisiSayisi
                 << " | Toplam: " << r.d.toplamUcret << "\n";
        }
    }
    in.close();
}

void DugunAra() {
    int no = IntAl("\nAranacak Dugun No: ", 1, 999999999);
    DugunBulS s = DugunBul(no);
    if (!s.bulundu) { cout << "Dugun bulunamadi.\n"; return; }

    Dugun d = s.d;
    cout << "\n--- DUGUN BILGILERI ---\n";
    cout << "No: " << d.dugunNo << "\n";
    cout << "Tarih/Saat: " << d.tarih << " " << d.saat << "\n";
    cout << "Damat: " << d.damatAdi << " (Baba: " << d.damatBabaAdi << ")\n";
    cout << "Gelin: " << d.gelinAdi << " (Anne: " << d.gelinAnneAdi << ")\n";
    cout << "Patron: " << d.patronAdi << " Tel: " << d.patronTel << "\n";
    cout << "Damat Tel: " << d.damatTel << "\n";
    cout << "SalonNo: " << d.salonNo << " | Kisi: " << d.kisiSayisi << "\n";
    cout << "SalonUcreti: " << d.salonUcreti << " | ToplamUcret: " << d.toplamUcret << "\n";
}

void DugunGuncelle() {
    int no = IntAl("\nGuncellenecek Dugun No: ", 1, 999999999);
    DugunBulS s = DugunBul(no);
    if (!s.bulundu) { cout << "Dugun bulunamadi.\n"; return; }

    Dugun yeni = s.d;
    cout << "Bos birakirsan DEGISTIRMEZ.\n";

    string tmp;

    tmp = TarihAlBos("Yeni tarih (YYYY-MM-DD): ");
    if (tmp != "") yeni.tarih = tmp;

    tmp = SaatAlBos("Yeni saat  (HH:MM): ");
    if (tmp != "") yeni.saat = tmp;

    tmp = SatirAl("Yeni patron adi: ", 1);
    if (tmp != "") yeni.patronAdi = tmp;

    tmp = TelAlBos("Yeni patron tel: ");
    if (tmp != "") yeni.patronTel = tmp;

    tmp = TelAlBos("Yeni damat tel: ");
    if (tmp != "") yeni.damatTel = tmp;

    tmp = SatirAl("Yeni salon no: ", 1);
    if (tmp != "") {
        IntSonuc p = IntParse(tmp);
        if (p.ok && p.deger > 0) yeni.salonNo = p.deger;
        else cout << "Salon no gecersiz, degistirilmedi.\n";
    }

    tmp = SatirAl("Yeni kisi sayisi: ", 1);
    if (tmp != "") {
        IntSonuc p = IntParse(tmp);
        if (p.ok && p.deger > 0) yeni.kisiSayisi = p.deger;
        else cout << "Kisi sayisi gecersiz, degistirilmedi.\n";
    }

    tmp = SatirAl("Yeni salon ucreti: ", 1);
    if (tmp != "") {
        DblSonuc p = DoubleParse(tmp);
        if (p.ok && p.deger >= 0) yeni.salonUcreti = p.deger;
        else cout << "Salon ucreti gecersiz, degistirilmedi.\n";
    }

    // Toplam ücret salon ücretinden küçük olmasın
    if (yeni.toplamUcret < yeni.salonUcreti) yeni.toplamUcret = yeni.salonUcreti;

    // Dosyayı baştan yaz (tmp dosya yöntemi)
    ifstream in(DUGUN_DOSYA);
    ofstream out("tmp_dugun.txt");
    string satir;

    while (getline(in, satir)) {
        DugunOkuSonuc r = DugunSatirOku(satir);
        if (r.ok && r.d.dugunNo == no) out << DugunSatirYap(yeni) << "\n";
        else out << satir << "\n";
    }
    in.close();
    out.close();

    remove(DUGUN_DOSYA.c_str());
    rename("tmp_dugun.txt", DUGUN_DOSYA.c_str());

    cout << "Dugun guncellendi.\n";
}

void DugunSilVeIliskiliTemizle() {
    int no = IntAl("\nSilinecek Dugun No: ", 1, 999999999);
    if (!DugunVarMi(no)) { cout << "Dugun bulunamadi.\n"; return; }

    // 1-Dugunler.txt içinden sil
    {
        ifstream in(DUGUN_DOSYA);
        ofstream out("tmp_dugun.txt");
        string satir;
        while (getline(in, satir)) {
            DugunOkuSonuc r = DugunSatirOku(satir);
            if (r.ok && r.d.dugunNo == no) {
                // yazma
            } else {
                out << satir << "\n";
            }
        }
        in.close(); out.close();
        remove(DUGUN_DOSYA.c_str());
        rename("tmp_dugun.txt", DUGUN_DOSYA.c_str());
    }

    // 2-Davetliler.txt içinden bu dugunNo olanları sil
    {
        ifstream in(DAVETLI_DOSYA);
        ofstream out("tmp_davetli.txt");
        if (in.is_open()) {
            string satir;
            while (getline(in, satir)) {
                DavetliOkuSonuc r = DavetliSatirOku(satir);
                if (r.ok && r.v.dugunNo == no) {
                    // yazma
                } else {
                    out << satir << "\n";
                }
            }
            in.close(); out.close();
            remove(DAVETLI_DOSYA.c_str());
            rename("tmp_davetli.txt", DAVETLI_DOSYA.c_str());
        } else {
            out.close();
            remove("tmp_davetli.txt");
        }
    }

    // 3- Masraf.txt içinden bu dugunNo olanları sil
    {
        ifstream in(MASRAF_DOSYA);
        ofstream out("tmp_masraf.txt");
        if (in.is_open()) {
            string satir;
            while (getline(in, satir)) {
                MasrafOkuSonuc r = MasrafSatirOku(satir);
                if (r.ok && r.m.dugunNo == no) {
                    // yazma
                } else {
                    out << satir << "\n";
                }
            }
            in.close(); out.close();
            remove(MASRAF_DOSYA.c_str());
            rename("tmp_masraf.txt", MASRAF_DOSYA.c_str());
        } else {
            out.close();
            remove("tmp_masraf.txt");
        }
    }

    // 4- Masalar.txt içinden bu dugunNo olan masa kayıtlarını sil
    {
        ifstream in(MASA_RAPOR);
        ofstream out("tmp_masa.txt");
        if (in.is_open()) {
            string satir;
            while (getline(in, satir)) {
                MasaOkuSonuc r = MasaSatirOku(satir);
                if (r.ok && r.k.dugunNo == no) {
                    // yazma
                } else {
                    out << satir << "\n";
                }
            }
            in.close(); out.close();
            remove(MASA_RAPOR.c_str());
            rename("tmp_masa.txt", MASA_RAPOR.c_str());
        } else {
            out.close();
            remove("tmp_masa.txt");
        }
    }

    cout << "Dugun ve iliskili davetli/masraf/masa kayitlari silindi.\n";
}



// ======================== Davetli işlemleri ========================

void DavetliEkle() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    if (!DugunVarMi(dugunNo)) { cout << "Once bu DugunNo ile dugun ekleyin.\n"; return; }

    Davetli v;
    v.dugunNo = dugunNo;
    v.davetliNo = IntAl("Davetli No: ", 1, 999999999);

    if (DavetliVarMi(v.dugunNo, v.davetliNo)) { cout << "Bu davetli zaten var.\n"; return; }

    v.ad = SatirAl("Ad: ", 0);
    v.soyad = SatirAl("Soyad: ", 0);
    v.taraf = CharSecimAl("Taraf (D=damat, G=gelin): ", 'D', 'G');
    v.adres = SatirAl("Adres: ", 0);
    v.tel = TelAl("Telefon (+905.. veya 05..): ");
    v.durum = CharSecimAl("Durum (E=gelecek, H=gelemeyecek): ", 'E', 'H');

    ofstream out(DAVETLI_DOSYA, ios::app);
    out << DavetliSatirYap(v) << "\n";
    out.close();

    cout << "Davetli eklendi.\n";
}

void DavetliSil() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    int davNo   = IntAl("Davetli No: ", 1, 999999999);

    if (!DavetliVarMi(dugunNo, davNo)) { cout << "Davetli bulunamadi.\n"; return; }

    ifstream in(DAVETLI_DOSYA);
    ofstream out("tmp_davetli.txt");

    string satir;
    while (getline(in, satir)) {
        DavetliOkuSonuc r = DavetliSatirOku(satir);
        if (r.ok && r.v.dugunNo == dugunNo && r.v.davetliNo == davNo) {
            // yazma
        } else {
            out << satir << "\n";
        }
    }
    in.close(); out.close();

    remove(DAVETLI_DOSYA.c_str());
    rename("tmp_davetli.txt", DAVETLI_DOSYA.c_str());

    cout << "Davetli silindi.\n";
}

void DavetliGuncelle() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    int davNo   = IntAl("Davetli No: ", 1, 999999999);

    if (!DavetliVarMi(dugunNo, davNo)) { cout << "Davetli bulunamadi.\n"; return; }

    cout << "Bos birakirsan DEGISTIRMEZ.\n";

    ifstream in(DAVETLI_DOSYA);
    ofstream out("tmp_davetli.txt");

    string satir;
    while (getline(in, satir)) {
        DavetliOkuSonuc r = DavetliSatirOku(satir);
        if (r.ok && r.v.dugunNo == dugunNo && r.v.davetliNo == davNo) {
            Davetli v = r.v;

            string tmp;

            tmp = SatirAl("Yeni ad: ", 1);
            if (tmp != "") v.ad = tmp;

            tmp = SatirAl("Yeni soyad: ", 1);
            if (tmp != "") v.soyad = tmp;

            tmp = SatirAl("Yeni adres: ", 1);
            if (tmp != "") v.adres = tmp;

            tmp = TelAlBos("Yeni tel: ");
            if (tmp != "") v.tel = tmp;

            tmp = SatirAl("Yeni taraf (D/G) bos=degismez: ", 1);
            if (tmp == "D" || tmp == "d") v.taraf = 'D';
            if (tmp == "G" || tmp == "g") v.taraf = 'G';

            tmp = SatirAl("Yeni durum (E/H) bos=degismez: ", 1);
            if (tmp == "E" || tmp == "e") v.durum = 'E';
            if (tmp == "H" || tmp == "h") v.durum = 'H';

            out << DavetliSatirYap(v) << "\n";
        } else {
            out << satir << "\n";
        }
    }

    in.close(); out.close();
    remove(DAVETLI_DOSYA.c_str());
    rename("tmp_davetli.txt", DAVETLI_DOSYA.c_str());

    cout << "Davetli guncellendi.\n";
}

void DavetliRapor() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);

    ifstream in(DAVETLI_DOSYA);
    if (!in.is_open()) { cout << "Davetliler.txt yok.\n"; return; }

    cout << "\n--- GELECEKLER (E) ---\n";
    string satir;
    while (getline(in, satir)) {
        DavetliOkuSonuc r = DavetliSatirOku(satir);
        if (r.ok && r.v.dugunNo == dugunNo && r.v.durum == 'E') {
            cout << r.v.davetliNo << " | " << r.v.ad << " " << r.v.soyad
                 << " | Taraf:" << r.v.taraf << " | Tel:" << r.v.tel << "\n";
        }
    }

    in.clear();
    in.seekg(0, ios::beg);

    cout << "\n--- GELEMEYECEKLER (H) ---\n";
    while (getline(in, satir)) {
        DavetliOkuSonuc r = DavetliSatirOku(satir);
        if (r.ok && r.v.dugunNo == dugunNo && r.v.durum == 'H') {
            cout << r.v.davetliNo << " | " << r.v.ad << " " << r.v.soyad
                 << " | Taraf:" << r.v.taraf << " | Tel:" << r.v.tel << "\n";
        }
    }

    in.close();
}



// ======================== Masraf işlemleri ========================

void MasrafEkle() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    if (!DugunVarMi(dugunNo)) { cout << "Once dugun ekleyin.\n"; return; }

    Masraf m;
    m.dugunNo = dugunNo;
    m.kalem = SatirAl("Kalem (or: Tavuk Menu): ", 0);
    m.kategori = SatirAl("Kategori (Yemek/Aksesuar/Sanatci): ", 0);
    m.adet = IntAl("Adet: ", 1, 1000000);
    m.birimFiyat = DoubleAl("Birim fiyat: ", 0);

    ofstream out(MASRAF_DOSYA, ios::app);
    out << MasrafSatirYap(m) << "\n";
    out.close();

    cout << "Masraf eklendi.\n";
}

// Masraflari diziye al + toplamını hesapla (sadece verilen dugunNo)
struct MasrafListeSonuc {
    int n;
    double toplam;
};

MasrafListeSonuc MasraflariGetir(int dugunNo, Masraf dizi[], int maxN) {
    MasrafListeSonuc sonuc;
    sonuc.n = 0;
    sonuc.toplam = 0;

    ifstream in(MASRAF_DOSYA);
    if (!in.is_open()) return sonuc;

    string satir;
    while (getline(in, satir)) {
        MasrafOkuSonuc okunan = MasrafSatirOku(satir);
        if (okunan.ok && okunan.m.dugunNo == dugunNo) {
            if (sonuc.n < maxN) {
                dizi[sonuc.n++] = okunan.m;
                sonuc.toplam += (okunan.m.adet * okunan.m.birimFiyat);
            }
        }
    }
    in.close();
    return sonuc;
}

// Masraflari ekrana numaralı şekilde yazdırır
void MasraflariListele(int dugunNo) {
    Masraf dizi[5000];
    MasrafListeSonuc sonuc = MasraflariGetir(dugunNo, dizi, 5000);

    if (sonuc.n == 0) {
        cout << "Bu dugunNo icin masraf yok.\n";
        return;
    }

    cout << "\n--- MASRAF LISTESI (DugunNo: " << dugunNo << ") ---\n";
    for (int i = 0; i < sonuc.n; i++) {
        double tutar = dizi[i].adet * dizi[i].birimFiyat;
        cout << (i + 1) << ") "
             << dizi[i].kalem << " | " << dizi[i].kategori
             << " | Adet: " << dizi[i].adet
             << " | Birim: " << dizi[i].birimFiyat
             << " | Tutar: " << tutar << "\n";
    }
    cout << "Masraf Toplami: " << sonuc.toplam << "\n";
}

// Seçilen dugunNo'da masraf satırını (1..k) ile siler
void MasrafSil() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    if (!DugunVarMi(dugunNo)) { cout << "Dugun bulunamadi.\n"; return; }

    // Önce listeleyelim ki kullanıcı seçim yapsın
    Masraf dizi[5000];
    MasrafListeSonuc sonuc = MasraflariGetir(dugunNo, dizi, 5000);

    if (sonuc.n == 0) { cout << "Bu dugunNo icin silinecek masraf yok.\n"; return; }

    cout << "\nSilinecek masrafi secin:\n";
    for (int i = 0; i < sonuc.n; i++) {
        double tutar = dizi[i].adet * dizi[i].birimFiyat;
        cout << (i + 1) << ") " << dizi[i].kalem << " | " << dizi[i].kategori
             << " | " << dizi[i].adet << " x " << dizi[i].birimFiyat
             << " = " << tutar << "\n";
    }

    int secim = IntAl("Secim (1.." + to_string(sonuc.n) + "): ", 1, sonuc.n);

    // Dosyayı tmp ile yeniden yaz: seçilen kaydı atla
    ifstream in(MASRAF_DOSYA);
    if (!in.is_open()) { cout << "Masraf.txt yok.\n"; return; }

    ofstream out("tmp_masraf.txt");
    string satir;
    int sayac = 0;

    while (getline(in, satir)) {
        MasrafOkuSonuc okunan = MasrafSatirOku(satir);

        if (okunan.ok && okunan.m.dugunNo == dugunNo) {
            sayac++;
            if (sayac == secim) {
                // bu satır silinecek -> yazma
            } else {
                out << satir << "\n";
            }
        } else {
            // başka düğün veya bozuk satır -> koru
            out << satir << "\n";
        }
    }

    in.close();
    out.close();
    remove(MASRAF_DOSYA.c_str());
    rename("tmp_masraf.txt", MASRAF_DOSYA.c_str());

    cout << "Masraf silindi.\n";
}

// Seçilen dugunNo'da masraf satırını (1..k) ile günceller
void MasrafGuncelle() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    if (!DugunVarMi(dugunNo)) { cout << "Dugun bulunamadi.\n"; return; }

    Masraf dizi[5000];
    MasrafListeSonuc sonuc = MasraflariGetir(dugunNo, dizi, 5000);

    if (sonuc.n == 0) { cout << "Bu dugunNo icin guncellenecek masraf yok.\n"; return; }

    cout << "\nGuncellenecek masrafi secin:\n";
    for (int i = 0; i < sonuc.n; i++) {
        double tutar = dizi[i].adet * dizi[i].birimFiyat;
        cout << (i + 1) << ") " << dizi[i].kalem << " | " << dizi[i].kategori
             << " | " << dizi[i].adet << " x " << dizi[i].birimFiyat
             << " = " << tutar << "\n";
    }

    int secim = IntAl("Secim (1.." + to_string(sonuc.n) + "): ", 1, sonuc.n);

    cout << "\nBos birakirsan DEGISTIRMEZ.\n";
    string yeniKalem = SatirAl("Yeni kalem: ", 1);
    string yeniKategori = SatirAl("Yeni kategori: ", 1);
    string yeniAdetStr = SatirAl("Yeni adet: ", 1);
    string yeniBirimStr = SatirAl("Yeni birim fiyat: ", 1);

    // Dosyayı tmp ile yeniden yaz: seçilen kaydı değiştir
    ifstream in(MASRAF_DOSYA);
    if (!in.is_open()) { cout << "Masraf.txt yok.\n"; return; }

    ofstream out("tmp_masraf.txt");
    string satir;
    int sayac = 0;

    while (getline(in, satir)) {
        MasrafOkuSonuc okunan = MasrafSatirOku(satir);

        if (okunan.ok && okunan.m.dugunNo == dugunNo) {
            sayac++;
            if (sayac == secim) {
                Masraf yeni = okunan.m;

                if (yeniKalem != "") yeni.kalem = yeniKalem;
                if (yeniKategori != "") yeni.kategori = yeniKategori;

                if (yeniAdetStr != "") {
                    IntSonuc adetSonuc = IntParse(yeniAdetStr);
                    if (adetSonuc.ok && adetSonuc.deger > 0) yeni.adet = adetSonuc.deger;
                    else cout << "Adet gecersiz, degistirilmedi.\n";
                }

                if (yeniBirimStr != "") {
                    DblSonuc fiyatSonuc = DoubleParse(yeniBirimStr);
                    if (fiyatSonuc.ok && fiyatSonuc.deger >= 0) yeni.birimFiyat = fiyatSonuc.deger;
                    else cout << "Birim fiyat gecersiz, degistirilmedi.\n";
                }

                out << MasrafSatirYap(yeni) << "\n";
            } else {
                out << satir << "\n";
            }
        } else {
            out << satir << "\n";
        }
    }

    in.close();
    out.close();
    remove(MASRAF_DOSYA.c_str());
    rename("tmp_masraf.txt", MASRAF_DOSYA.c_str());

    cout << "Masraf guncellendi.\n";
}

// Bir düğünün masraf toplamını hesaplar
double MasrafToplam(int dugunNo) {
    ifstream in(MASRAF_DOSYA);
    if (!in.is_open()) return 0;

    double toplam = 0;
    string satir;
    while (getline(in, satir)) {
        MasrafOkuSonuc r = MasrafSatirOku(satir);
        if (r.ok && r.m.dugunNo == dugunNo) {
            toplam = toplam + (r.m.adet * r.m.birimFiyat);
        }
    }
    in.close();
    return toplam;
}

// Toplam ücreti (salon + masraflar) hesaplayıp Dugunler.txt içinde günceller
void ToplamUcretHesaplaVeGuncelle() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    DugunBulS bulunan = DugunBul(dugunNo);
    if (!bulunan.bulundu) { cout << "Dugun bulunamadi.\n"; return; }

    // 1-Masrafları tek tek yazdır
    ifstream inMasraf(MASRAF_DOSYA);
    double ekMasrafToplam = 0;

    cout << "\n--- UCRET DOKUMU ---\n";
    cout << "Salon Ucreti: " << bulunan.d.salonUcreti << "\n";

    if (!inMasraf.is_open()) {
        cout << "\nMasraf.txt yok. Ek masraf = 0 kabul edildi.\n";
    } else {
        cout << "\n--- EK MASRAFLAR ---\n";
        string satir;
        int sira = 0;

        // Kategori toplamları için basit dizi (max 50 kategori)
        string katAd[50];
        double katToplam[50];
        int katN = 0;

        while (getline(inMasraf, satir)) {
            MasrafOkuSonuc okunan = MasrafSatirOku(satir);
            if (okunan.ok && okunan.m.dugunNo == dugunNo) {
                sira++;
                double tutar = okunan.m.adet * okunan.m.birimFiyat;
                ekMasrafToplam += tutar;

                cout << sira << ") "
                     << okunan.m.kalem << " | " << okunan.m.kategori
                     << " | Adet: " << okunan.m.adet
                     << " | Birim: " << okunan.m.birimFiyat
                     << " | Tutar: " << tutar << "\n";

                // kategori biriktir
                int bulunduMu = 0;
                for (int i = 0; i < katN; i++) {
                    if (katAd[i] == okunan.m.kategori) {
                        katToplam[i] += tutar;
                        bulunduMu = 1;
                        break;
                    }
                }
                if (!bulunduMu && katN < 50) {
                    katAd[katN] = okunan.m.kategori;
                    katToplam[katN] = tutar;
                    katN++;
                }
            }
        }
        inMasraf.close();

        if (sira == 0) cout << "(Bu dugun icin ek masraf yok)\n";

        // kategori özeti
        if (katN > 0) {
            cout << "\n--- KATEGORI OZETI ---\n";
            for (int i = 0; i < katN; i++) {
                cout << katAd[i] << ": " << katToplam[i] << "\n";
            }
        }
    }

    // 2-Toplam ücret
    Dugun yeni = bulunan.d;
    yeni.toplamUcret = yeni.salonUcreti + ekMasrafToplam;

    // 3-Dugunler.txt içinde güncelle
    ifstream inDugun(DUGUN_DOSYA);
    ofstream outDugun("tmp_dugun.txt");
    string satirD;

    while (getline(inDugun, satirD)) {
        DugunOkuSonuc okunanD = DugunSatirOku(satirD);
        if (okunanD.ok && okunanD.d.dugunNo == dugunNo) outDugun << DugunSatirYap(yeni) << "\n";
        else outDugun << satirD << "\n";
    }

    inDugun.close();
    outDugun.close();
    remove(DUGUN_DOSYA.c_str());
    rename("tmp_dugun.txt", DUGUN_DOSYA.c_str());

    cout << "\nEk Masraf Toplami: " << ekMasrafToplam << "\n";
    cout << "GENEL TOPLAM     : " << yeni.toplamUcret << "\n";
    cout << "(Dugunler.txt icinde toplam ucret guncellendi)\n";
}



// ======================== Masa planı (Masalar.txt tam veri kaydı) ========================

// Aynı dugunNo'ya ait eski masa kayıtlarını siler (diğer düğünler kalır)
void MasalarDugunPlaniniSil(int dugunNo) {
    ifstream in(MASA_RAPOR);
    if (!in.is_open()) return; // dosya yoksa sorun değil

    ofstream out("tmp_masa.txt");
    string satir;

    while (getline(in, satir)) {
        MasaOkuSonuc r = MasaSatirOku(satir);

        // Eğer satır parse oluyorsa ve bu düğüne aitse -> sil (yazma)
        if (r.ok && r.k.dugunNo == dugunNo) {
            // yazma
        } else {
            // Diğer düğün veya bozuk/boş satır -> aynen koru
            out << satir << "\n";
        }
    }

    in.close();
    out.close();

    remove(MASA_RAPOR.c_str());
    rename("tmp_masa.txt", MASA_RAPOR.c_str());
}

// Planı Masalar.txt dosyasına yazar
void MasaPlaniDosyayaYaz(int dugunNo, int kapasite, Davetli damat[], int damatSay, Davetli gelin[], int gelinSay) {
    ofstream out(MASA_RAPOR, ios::app);
    if (!out.is_open()) { cout << "Masalar.txt acilamadi.\n"; return; }

    // Masa numaralandırma: Damat tarafı masaları 1'den başlar, Gelin tarafı masaları damat bitince devam eder.
    int masaNo = 1;

    // Damat tarafı
    int i = 0;
    while (i < damatSay) {
        int k = 0;
        while (k < kapasite && i < damatSay) {
            MasaKaydi kayit;
            kayit.dugunNo = dugunNo;
            kayit.kapasite = kapasite;
            kayit.masaNo = masaNo;
            kayit.taraf = 'D';
            kayit.davetliNo = damat[i].davetliNo;
            kayit.ad = damat[i].ad;
            kayit.soyad = damat[i].soyad;

            out << MasaSatirYap(kayit) << "\n";

            i++; k++;
        }
        masaNo++;
    }

    // Gelin tarafı
    i = 0;
    while (i < gelinSay) {
        int k = 0;
        while (k < kapasite && i < gelinSay) {
            MasaKaydi kayit;
            kayit.dugunNo = dugunNo;
            kayit.kapasite = kapasite;
            kayit.masaNo = masaNo;
            kayit.taraf = 'G';
            kayit.davetliNo = gelin[i].davetliNo;
            kayit.ad = gelin[i].ad;
            kayit.soyad = gelin[i].soyad;

            out << MasaSatirYap(kayit) << "\n";

            i++; k++;
        }
        masaNo++;
    }

    out.close();
}

// Seçilen düğünün masa planını dosyadan okuyup ekrana basar
void MasalarDosyasiniGoster(int dugunNo) {
    ifstream in(MASA_RAPOR);
    if (!in.is_open()) { cout << "Masalar.txt yok.\n"; return; }

    // Bu düğüne ait kayıtları diziye alalım
    MasaKaydi kayitlar[6000];
    int n = 0;
    int kapasite = 0;

    string satir;
    while (getline(in, satir)) {
        MasaOkuSonuc r = MasaSatirOku(satir);
        if (r.ok && r.k.dugunNo == dugunNo) {
            if (kapasite == 0) kapasite = r.k.kapasite; // ilk kayıttan kapasiteyi al
            if (n < 6000) kayitlar[n++] = r.k;
        }
    }
    in.close();

    if (n == 0) {
        cout << "Bu dugunNo icin Masalar.txt icinde plan bulunamadi.\n";
        return;
    }

    cout << "\n======== MASA PLANI ========\n";
    cout << "DugunNo: " << dugunNo << " | Kapasite: " << kapasite << "\n";

    // 1-Damat tarafını yaz
    cout << "\n=== DAMAT TARAFI MASALARI ===\n";
    int yazildi = 0;
    int oncekiMasa = -1;

    for (int i = 0; i < n; i++) {
        if (kayitlar[i].taraf != 'D') continue;

        if (kayitlar[i].masaNo != oncekiMasa) {
            cout << "Masa " << kayitlar[i].masaNo << " (D)\n";
            oncekiMasa = kayitlar[i].masaNo;
        }

        cout << kayitlar[i].davetliNo << " | " << kayitlar[i].ad << " " << kayitlar[i].soyad << "\n";
        yazildi = 1;
    }
    if (!yazildi) cout << "(Damat tarafinda kayit yok)\n";

    // 2-Gelin tarafını yaz
    cout << "\n=== GELIN TARAFI MASALARI ===\n";
    yazildi = 0;
    oncekiMasa = -1;

    for (int i = 0; i < n; i++) {
        if (kayitlar[i].taraf != 'G') continue;

        if (kayitlar[i].masaNo != oncekiMasa) {
            cout << "Masa " << kayitlar[i].masaNo << " (G)\n";
            oncekiMasa = kayitlar[i].masaNo;
        }

        cout << kayitlar[i].davetliNo << " | " << kayitlar[i].ad << " " << kayitlar[i].soyad << "\n";
        yazildi = 1;
    }
    if (!yazildi) cout << "(Gelin tarafinda kayit yok)\n";
}

// Masa planını oluşturur
void MasaPlaniOlustur() {
    int dugunNo = IntAl("\nDugun No: ", 1, 999999999);
    if (!DugunVarMi(dugunNo)) { cout << "Dugun bulunamadi.\n"; return; }

    int kapasite = IntAl("Masa kapasitesi (or: 8): ", 1, 50);

    // En fazla 2000'er davetli (D/G) alalım
    Davetli damat[2000];
    Davetli gelin[2000];
    int damatSay = 0, gelinSay = 0;

    ifstream in(DAVETLI_DOSYA);
    if (!in.is_open()) { cout << "Davetliler.txt yok.\n"; return; }

    // Sadece bu düğüne ait ve durumu E olanlar alınır
    string satir;
    while (getline(in, satir)) {
        DavetliOkuSonuc r = DavetliSatirOku(satir);
        if (r.ok && r.v.dugunNo == dugunNo && r.v.durum == 'E') {
            if (r.v.taraf == 'D' && damatSay < 2000) damat[damatSay++] = r.v;
            if (r.v.taraf == 'G' && gelinSay < 2000) gelin[gelinSay++] = r.v;
        }
    }
    in.close();

    // Eğer hiç gelecek davetli yoksa: eski planı silip mesaj verelim
    if (damatSay == 0 && gelinSay == 0) {
        MasalarDugunPlaniniSil(dugunNo);
        cout << "Bu dugun icin GELECEK (E) davetli yok. Masa plani olusmadi.\n";
        return;
    }

    // Aynı dugunNo'nun eski planını sil (diğer düğünlerin planı kalır)
    MasalarDugunPlaniniSil(dugunNo);

    // Yeni planı dosyaya yaz
    MasaPlaniDosyayaYaz(dugunNo, kapasite, damat, damatSay, gelin, gelinSay);

    // Ekrana basmak için dosyadan oku ve yazdır
    MasalarDosyasiniGoster(dugunNo);
}



// ======================== MAIN ========================

int main() {
    EkraniTemizle();

    int secim = -1;

    while (secim != 0) {
        cout << "\n===== MENU =====\n\n";
        cout << "1) Dugun Ekle\n";
        cout << "2) Dugun Listele\n";
        cout << "3) Dugun Ara\n";
        cout << "4) Dugun Guncelle\n";
        cout << "5) Dugun Sil\n";
        cout << "6) Davetli Ekle\n";
        cout << "7) Davetli Sil\n";
        cout << "8) Davetli Guncelle\n";
        cout << "9) Davetli Rapor (E/H)\n";
        cout << "10) Masraf Ekle\n";
        cout << "11) Masraf Sil\n";
        cout << "12) Masraf Guncelle\n";
        cout << "13) Toplam Ucret Hesapla ve Guncelle\n";
        cout << "14) Masa Plani Olustur\n";
        cout << "15) Masa Plani Yazdir\n";
        cout << "0) Cikis\n";

        secim = IntAl("\nSecim: ", 0, 15);

        EkraniTemizle();

        if      (secim == 1)  { DugunEkle(); IslemSonuTemizle(); }
        else if (secim == 2)  { DugunListele(); IslemSonuTemizle(); }
        else if (secim == 3)  { DugunAra(); IslemSonuTemizle(); }
        else if (secim == 4)  { DugunGuncelle(); IslemSonuTemizle(); }
        else if (secim == 5)  { DugunSilVeIliskiliTemizle(); IslemSonuTemizle(); }
        else if (secim == 6)  { DavetliEkle(); IslemSonuTemizle(); }
        else if (secim == 7)  { DavetliSil(); IslemSonuTemizle(); }
        else if (secim == 8)  { DavetliGuncelle(); IslemSonuTemizle(); }
        else if (secim == 9)  { DavetliRapor(); IslemSonuTemizle(); }
        else if (secim == 10) { MasrafEkle(); IslemSonuTemizle(); }
        else if (secim == 11) { MasrafSil(); IslemSonuTemizle(); }
        else if (secim == 12) { MasrafGuncelle(); IslemSonuTemizle(); }
        else if (secim == 13) { ToplamUcretHesaplaVeGuncelle(); IslemSonuTemizle(); }
        else if (secim == 14) { MasaPlaniOlustur(); IslemSonuTemizle(); }
        else if (secim == 15) {
            int no = IntAl("Yazdirilacak Dugun No: ", 1, 999999999);
            MasalarDosyasiniGoster(no);
            IslemSonuTemizle();
        }
        else if (secim == 0)  { /* çıkış */ }
    }

    return 0;
}