# PetaLinux 기반 HLS 가속기 (ZCU104)

AMD/Xilinx **ZCU104** 보드에서 Vitis HLS로 만든 **행렬 곱셈(Matrix Multiplication) 가속기**를 PetaLinux 임베디드 리눅스와 통합하고, Linux 유저스페이스(C++/Python)에서 UIO로 직접 제어하는 튜토리얼 실습 프로젝트입니다.

- **대상 보드:** Xilinx ZCU104 (Zynq UltraScale+ MPSoC, `xczu7ev-ffvc1156-2-e`)
- **툴 버전:** Vivado / Vitis HLS / PetaLinux 전부 **2024.1**
- **개발 환경:** 네이티브 Ubuntu 24.04 LTS (WSL/Windows 미사용)
- **원본 튜토리얼:** [Xilinx-PetaLinux-Based-FPGA-Acceleration-Design-Tutorial](https://github.com/vandinhtranengg/Xilinx-PetaLinux-Based-FPGA-Acceleration-Design-Tutorial/)

## 아키텍처 한눈에 보기

```
Linux Userspace App (C++ / Python)
        | mmap()
Linux Kernel — UIO 드라이버 (mmult-control, mmult-buffer)
        | AXI-Lite 제어              | DDR 직접 접근 (DMA)
PS (ARM Cortex-A53)  <——>  PL (FPGA: HLS 행렬곱셈 가속기)
```

- **제어 경로:** CPU → AXI-Lite (`0xA0000000`, 64KiB) → 가속기 START/DONE 레지스터
- **데이터 경로:** 가속기 → AXI Master → 예약된 물리 DDR (`0x70000000`, 16MiB) 직접 R/W

## 폴더 구조

```
petalinux_accel/
├── hls/            HLS 가속기 소스 (mmult.cpp, 테스트벤치)
├── app/            유저스페이스 제어 앱 (C++ / Python)
├── docs/           튜토리얼 원문(PDF) 및 한글 정리 노트
├── mmult_hls/       Vitis HLS 프로젝트  (git 미추적, 로컬 생성물)
├── mmult_zcu104/    Vivado 프로젝트     (git 미추적, 로컬 생성물)
└── project/         PetaLinux 프로젝트  (git 미추적, 로컬 생성물)
```

`mmult_hls/`, `mmult_zcu104/`, `project/`는 각각 Vitis HLS, Vivado, PetaLinux 툴이 생성하는 대용량 빌드 산출물(수십GB)이라 저장소에는 올리지 않고 로컬에만 둡니다. 소스(`hls/`, `app/`)와 문서(`docs/`)만으로 전체 워크플로우를 다시 재현할 수 있습니다.

## 진행 순서

1. Vitis HLS로 `hls/mmult.cpp` 합성 → IP 패키징
2. Vivado에서 ZynqMP PS와 가속기 IP 연결 → 비트스트림/XSA 생성
3. PetaLinux 프로젝트에 XSA 임포트 → 디바이스 트리 자동 생성
4. `system-user.dtsi`로 UIO 노출 + DDR 메모리 예약
5. 커널 UIO 설정, rootfs 구성, `app/`의 C++/Python 앱을 rootfs에 포함
6. 전체 빌드 → SD카드 부팅 → 보드에서 가속기 실행/검증

자세한 절차는 [`docs/PetaLinux_tutorial_Note.md`](docs/PetaLinux_tutorial_Note.md)에 단계별로 정리되어 있습니다.
