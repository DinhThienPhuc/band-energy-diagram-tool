#### 1. Clone hay download repo.
#### 2. Để chạy chương trình chỉ cần chạy file main.c
- Form của file số liệu có dạng:
```
 Fermi (or HOMO) energy (hartree) =   0.20652   Average Vxc (hartree)=  -0.35454
 Eigenvalues (hartree) for nkpt=  10  k points:
 kpt#   1, nband=  5, wtk=  0.09375, kpt= -0.1250 -0.2500  0.0000 (reduced coord)
  -0.19906   0.09755   0.16774   0.17061   0.29409
 kpt#   2, nband=  5, wtk=  0.09375, kpt= -0.1250  0.5000  0.0000 (reduced coord)
  -0.13611  -0.03404   0.13186   0.15884   0.29574
 kpt#   3, nband=  5, wtk=  0.09375, kpt= -0.2500 -0.3750  0.0000 (reduced coord)
  -0.15984   0.03456   0.11690   0.13456   0.26383
 kpt#   4, nband=  5, wtk=  0.18750, kpt= -0.1250 -0.3750  0.1250 (reduced coord)
  -0.14384   0.00060   0.09257   0.14481   0.32117
 kpt#   5, nband=  5, wtk=  0.09375, kpt= -0.1250  0.2500  0.0000 (reduced coord)
  -0.18004   0.04537   0.13042   0.18839   0.31542
 kpt#   6, nband=  5, wtk=  0.09375, kpt= -0.2500  0.3750  0.0000 (reduced coord)
  -0.11733  -0.02919   0.06725   0.14664   0.28211
 kpt#   7, nband=  5, wtk=  0.09375, kpt= -0.3750  0.5000  0.0000 (reduced coord)
  -0.10387  -0.03536   0.08810   0.11684   0.24639
 kpt#   8, nband=  5, wtk=  0.18750, kpt= -0.2500  0.5000  0.1250 (reduced coord)
  -0.09705  -0.03929   0.06446   0.10596   0.30259
 kpt#   9, nband=  5, wtk=  0.03125, kpt= -0.1250  0.0000  0.0000 (reduced coord)
  -0.21910   0.15766   0.20652   0.20652   0.30730
 kpt#  10, nband=  5, wtk=  0.03125, kpt= -0.3750  0.0000  0.0000 (reduced coord)
  -0.16338  -0.00420   0.17667   0.17667   0.27609
```
- Cần nhập nband bằng tay cho đúng với file số liệu (có thể thay đổi tên file xuất ra cho dễ đọc) bằng cách mở file main.c và thay đổi giá trị của biến nBand
```
int main(int argc, char *argv[])
{
    FILE *fp;
    FILE *fw = fopen("tbase3_xo_DS2_EIG.txt", "w");   // Thay đổi tên fiel xuất ra
    int nBand = 5;                                    // Thay đổi nband thủ công
    char buf[BUF_SIZE];
    if (argc != 2)
    {
```
- Lưu file main.c. Lưu ý là file main.c và file số liệu để cùng cấp thư mục.
- Chạy chương trình bằng cú pháp: make main && ./main file_số_liệu
- Kết quả ta sẽ thu được file tbase3_xo_DS2_EIG.txt với các số liệu đã được parse.
- Sử dụng file này và phần mềm origin sẽ vẽ được đồ thị năng lượng ứng với từng trị riêng
