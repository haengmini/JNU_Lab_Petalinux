# ZCU104 기반 PetaLinux 제어 HLS 가속기 설계 및 배포 가이드 (PetaLinux-Controlled HLS Accelerator on ZCU104)

- **원작자:** TRAN VAN DINH (원작: 2026-05-23)
- **작성/정리:** 임베디드 시스템 설계 연구실 (Embedded System Design Lab - Department of Electronic Engineering - Jeju National University)
- **대상 플랫폼:** Xilinx / AMD ZCU104 평가 보드 (Zynq UltraScale+ MPSoC `xczu7ev-ffvc1156-2-e`)
- **사용 도구 버전:** Vivado 2024.1 / Vitis 2024.1 / Vitis HLS 2024.1 / PetaLinux 2024.1 / ZCU104 BSP 2024.1
- **원작자 GitHub 저장소:** [https://github.com/vandinhtranengg/Xilinx-PetaLinux-Based-FPGA-Acceleration-Design-Tutorial/](https://github.com/vandinhtranengg/Xilinx-PetaLinux-Based-FPGA-Acceleration-Design-Tutorial/)

---

## 목차 (Table of Contents)

1. [튜토리얼 목표 및 시스템 아키텍처 (Tutorial Objective &amp; Architecture)](#1-튜토리얼-목표-및-시스템-아키텍처)
2. [PetaLinux 설치 및 ZCU104 BSP 환경 준비 (Installing PetaLinux &amp; ZCU104 BSP Environment)](#2-petalinux-설치-및-zcu104-bsp-환경-준비)
  - 2.1 [필수 소프트웨어 및 파일 (Required Software &amp; Files)](#21-필수-소프트웨어-및-파일)
  - 2.2 [호스트 시스템 요구사항 (Host System Requirements)](#22-호스트-시스템-요구사항)
  - 2.3 [단일 네이티브 Linux 환경 구성](#23-단일-네이티브-linux-환경-구성)
  - 2.4 [Ubuntu 사전 패키지 설치 (Prepare Ubuntu)](#24-ubuntu-사전-패키지-설치)
  - 2.5 [로케일 설정 (Configure Locale)](#25-로케일-설정)
  - 2.6 [/bin/sh를 Bash로 변경_](#26-binsh를-bash로-변경)
  - 2.7 [PetaLinux 2024.1 인스톨러 다운로드](#27-petalinux-20241-인스톨러-다운로드)
  - 2.8 [PetaLinux 2024.1 설치 (일반 사용자 계정)](#28-petalinux-20241-설치)
  - 2.9 [설치 디렉토리 이동 금지 주의사항](#29-설치-디렉토리-이동-금지-주의사항)
  - 2.10 [PetaLinux 환경 변수 로드](#210-petalinux-환경-변수-로드)
  - 2.11 [PetaLinux BSP 개요](#211-petalinux-bsp-개요)
  - 2.12 [ZCU104 BSP 다운로드](#212-zcu104-bsp-다운로드)
  - 2.13 [BSP 파일 다운로드 위치 확인](#213-bsp-파일-다운로드-위치-확인)
  - 2.14 [BSP 기반 PetaLinux 프로젝트 생성](#214-bsp-기반-petalinux-프로젝트-생성)
  - 2.15 [ZCU104 부팅 체인 및 PetaLinux 빌드 구성 요소](#215-zcu104-부팅-체인-및-petalinux-빌드-구성-요소)
  - 2.16 [순정(Unmodified) BSP 초기 빌드 검증](#216-순정-bsp-초기-빌드-검증)
  - 2.17 [부팅 파일 생성 (BOOT.BIN)](#217-부팅-파일-생성-bootbin)
  - 2.18 [부팅 파일 SD 카드 복사 준비](#218-부팅-파일-sd-카드-복사-준비)
  - 2.19 [SD 카드 파티션 및 파일 준비](#219-sd-카드-파티션-및-파일-준비)
  - 2.20 [ZCU104 SD 카드 부팅 모드 스위치 설정 (SW6)](#220-zcu104-sd-카드-부팅-모드-스위치-설정)
  - 2.21 [UART 시리얼 터미널 설정](#221-uart-시리얼-터미널-설정)
  - 2.22 [PetaLinux 부팅 검증](#222-petalinux-부팅-검증)
  - 2.23 [개발 중 수정 항목별 교체 대상 SD 카드 파일 매트릭스](#223-개발-중-수정-항목별-교체-대상-sd-카드-파일-매트릭스)
  - 2.24 [권장 디렉토리 구조](#224-권장-디렉토리-구조)
3. [가속기 설계 참조 (Accelerator Design Reference)](#3-가속기-설계-참조)
4. [예제 가속기: 행렬 곱셈 (Example Accelerator: Matrix Multiplication)](#4-예제-가속기-행렬-곱셈)
  - 4.1 [가속기 인터페이스 요구사항](#41-가속기-인터페이스-요구사항)
  - 4.2 [Vitis HLS IP 빌드 및 검증](#42-vitis-hls-ip-빌드-및-검증)
5. [AXI-Lite 레지스터 맵 분석 (AXI-Lite Register Map)](#5-axi-lite-레지스터-맵-분석)
6. [Vivado 하드웨어 통합 (Vivado Hardware Integration)](#6-vivado-하드웨어-통합)
  - 6.1 [제어 경로 (Control Path)](#61-제어-경로)
  - 6.2 [데이터 경로 (Data Path)](#62-데이터-경로)
  - 6.3 [인터럽트 경로 (Interrupt Path)](#63-인터럽트-경로)
  - 6.4 [클럭 및 리셋 (Clock &amp; Reset)](#64-클럭-및-리셋)
7. [주소 할당, 비트스트림 생성 및 XSA 내보내기 (Assign Address, Bitstream &amp; Export XSA)](#7-주소-할당-비트스트림-생성-및-xsa-내보내기)
8. [하드웨어 설명 가져오기 및 PetaLinux 디바이스 트리 생성 확인 (Import XSA &amp; Device Tree)](#8-하드웨어-설명-가져오기-및-petalinux-디바이스-트리-생성-확인)
  - 8.1 [XSA 복사 (Vivado 프로젝트 → PetaLinux 프로젝트)](#81-xsa-복사-vivado-프로젝트--petalinux-프로젝트)
  - 8.2 [PetaLinux 환경 로드 및 프로젝트 이동](#82-petalinux-환경-로드-및-프로젝트-이동)
  - 8.3 [Vivado 하드웨어 설명 임포트](#83-vivado-하드웨어-설명-임포트)
  - 8.4 [DTG (Device Tree Generator) 설정 확인](#84-dtg-device-tree-generator-설정-확인)
  - 8.5 [MACHINE_NAME 확인 ((AUTO) 설정 유지)](#85-machine_name-확인-auto-설정-유지)
  - 8.6 [MACHINE_NAME 설정의 중요성](#86-machine_name-설정의-중요성)
  - 8.7 [기타 DTG 옵션 점검](#87-기타-dtg-옵션-점검)
  - 8.8 [설정 저장 및 종료](#88-설정-저장-및-종료)
  - 8.9 [머신 설정 검증](#89-머신-설정-검증)
  - 8.10 [디바이스 트리 독립 빌드](#810-디바이스-트리-독립-빌드)
  - 8.11 [가속기 노드 자동 탐지 확인](#811-가속기-노드-자동-탐지-확인)
  - 8.12 [새로운 XSA 임포트 후 디바이스 트리 생성 실패 시 대처법 (mrproper)](#812-새로운-xsa-임포트-후-디바이스-트리-생성-실패-시-대처법)
9. [UIO 접근 구성 및 가속기용 DDR 메모리 예약 (Configure UIO &amp; Reserve DDR Memory)](#9-uio-접근-구성-및-가속기용-ddr-메모리-예약)
  - 9.1 [가속기 제어 주소 영역](#91-가속기-제어-주소-영역)
  - 9.2 [UIO를 통한 가속기 제어 레지스터 노출 (system-user.dtsi)](#92-uio를-통한-가속기-제어-레지스터-노출)
  - 9.3 [DDR 메모리 예약(Reserved Memory)이 필수적인 이유](#93-ddr-메모리-예약이-필수적인-이유)
  - 9.4 [DDR 영역 예약 설정 (reserved-memory, no-map)](#94-ddr-영역-예약-설정)
  - 9.5 [예약된 DDR 영역을 UIO로 노출 (mmult-buffer)](#95-예약된-ddr-영역을-uio로-노출)
  - 9.6 [왜 malloc()을 가속기 버퍼로 직접 사용할 수 없는가?](#96-왜-malloc을-가속기-버퍼로-직접-사용할-수-없는가)
  - 9.7 [행렬 버퍼 레이아웃 (Matrix Buffer Layout)](#97-행렬-버퍼-레이아웃)
  - 9.8 [완전한 system-user.dtsi 소스 코드 및 3대 핵심 기능](#98-완전한-system-userdtsi-소스-코드-및-3대-핵심-기능)
  - 9.9 [제어 공간(Control Space) vs 데이터 공간(Data Space) 비교](#99-제어-공간-vs-데이터-공간-비교)
  - 9.10 [고정 예약 DDR 방식이 항상 필수적인가? (대안 메커니즘 분석)](#910-고정-예약-ddr-방식이-항상-필수적인가)
  - 9.11 [디바이스 트리 빌드 및 역컴파일(dtc)을 통한 검증](#911-디바이스-트리-빌드-및-역컴파일을-통한-검증)
10. [PetaLinux에서 Generic UIO 지원 활성화 (Enable Generic UIO Support)](#10-petalinux에서-generic-uio-지원-활성화)
  - 10.1 [리눅스 커널에서 UIO 활성화 (Built-in &lt;*&gt; 설정)](#101-리눅스-커널에서-uio-활성화)
    - 10.2 [최종 UIO 커널 설정 검증 및 bsp.cfg 오버라이드](#102-최종-uio-커널-설정-검증-및-bspcfg-오버라이드)
    - 10.3 [커널 부트 아규먼트에 Generic UIO 매핑 추가](#103-커널-부트-아규먼트에-generic-uio-매핑-추가)
    - 10.4 [최종 DTB 역컴파일을 통한 부트 아규먼트 검증](#104-최종-dtb-역컴파일을-통한-부트-아규먼트-검증)
11. [PetaLinux 루트 파일시스템 준비 (Prepare Root Filesystem)](#11-petalinux-루트-파일시스템-준비)
  - 11.1 [petalinux-image-minimal 설정](#111-petalinux-image-minimal-설정)
    - 11.2 [개발 편의를 위한 자동 Root 로그인(auto-login) 활성화](#112-개발-편의를-위한-자동-root-로그인-활성화)
12. [Linux 유저스페이스 애플리케이션: C++ 및 Python (Userspace Applications)](#12-linux-유저스페이스-애플리케이션-c-및-python)
  - 12.1 [공통 소프트웨어-하드웨어 인터페이스 및 UIO 탐색 흐름](#121-공통-소프트웨어-하드웨어-인터페이스-및-uio-탐색-흐름)
    - 12.2 [C++ 애플리케이션 레시피 생성 및 활성화](#122-c-애플리케이션-레시피-생성-및-활성화)
    - 12.3 [C++ 애플리케이션 빌드](#123-c-애플리케이션-빌드)
    - 12.4 [호스트(Ubuntu)에서 C++ 크로스 컴파일을 수행하는 이유](#124-호스트에서-c-크로스-컴파일을-수행하는-이유)
    - 12.5 [Python 애플리케이션 개요](#125-python-애플리케이션-개요)
    - 12.6 [Python 스크립트를 보드로 전달하는 방법 (SCP / SD 카드)](#126-python-스크립트를-보드로-전달하는-방법)
    - 12.7 [C++과 Python의 동일 메모리 맵 접근 구조](#127-c과-python의-동일-메모리-맵-접근-구조)
    - 12.8 [C++ vs Python 종합 비교](#128-c-vs-python-종합-비교)
13. [최종 시스템 빌드, 패키징 및 SD 카드 부팅 준비 (Final Build, Package &amp; Boot)](#13-최종-시스템-빌드-패키징-및-sd-카드-부팅-준비)
  - 13.1 [전체 PetaLinux 프로젝트 빌드](#131-전체-petalinux-프로젝트-빌드)
    - 13.2 [BOOT.BIN 생성](#132-bootbin-생성)
    - 13.3 [petalinux-package --boot 실행 필요 여부 판단 기준](#133-petalinux-package---boot-실행-필요-여부-판단-기준)
    - 13.4 [예제 1: 소프트웨어 전용 수정 시 업데이트 워크플로우](#134-예제-1-소프트웨어-전용-수정-시-업데이트-워크플로우)
    - 13.5 [예제 2: 하드웨어 수정 시 업데이트 워크플로우](#135-예제-2-하드웨어-수정-시-업데이트-워크플로우)
    - 13.6 [부팅 파일 SD 카드 복사](#136-부팅-파일-sd-카드-복사)
    - 13.7 [타깃 보드 부팅 및 콘솔 확인](#137-타깃-보드-부팅-및-콘솔-확인)
14. [ZCU104 보드 상에서 UIO 디바이스 및 메모리 매핑 검증 (Verify UIO Devices &amp; Memory Mapping)](#14-zcu104-보드-상에서-uio-디바이스-및-메모리-매핑-검증)
  - 14.1 [실행 중인 리눅스 시스템 확인](#141-실행-중인-리눅스-시스템-확인)
    - 14.2 [UIO 디바이스 노드 확인 (/dev/uio*)](#142-uio-디바이스-노드-확인)
    - 14.3 [이름 기반 UIO 디바이스 식별 스크립트](#143-이름-기반-uio-디바이스-식별-스크립트)
    - 14.4 [Generic UIO 드라이버 바인딩 확인](#144-generic-uio-드라이버-바인딩-확인)
    - 14.5 [각 UIO 매핑의 물리 주소 및 크기 확인](#145-각-uio-매핑의-물리-주소-및-크기-확인)
    - 14.6 [모든 UIO 매핑 일괄 검증 원라이너 스크립트](#146-모든-uio-매핑-일괄-검증-원라이너-스크립트)
    - 14.7 [런타임 디바이스 트리 노드(of_node) 검증](#147-런타임-디바이스-트리-노드-검증)
    - 14.8 [예약된 DDR 레이아웃 및 가상-물리 주소 매핑 검증](#148-예약된-ddr-레이아웃-및-가상-물리-주소-매핑-검증)
    - 14.9 [C++ 실행 바이너리 확인](#149-c-실행-바이너리-확인)
    - 14.10 [장애 진단 및 문제 해결 (Troubleshooting Guide)](#1410-장애-진단-및-문제-해결)
15. [가속기 실행, 검증 및 성능 측정 (Run, Validate &amp; Measure Performance)](#15-가속기-실행-검증-및-성능-측정)
  - 15.1 [유저스페이스 가속기 호출 8단계 시퀀스](#151-유저스페이스-가속기-호출-8단계-시퀀스)
    - 15.2 [C++ 가속기 제어 핵심 코드 및 동작 분석](#152-c-가속기-제어-핵심-코드-및-동작-분석)
    - 15.3 [Python 가속기 제어 핵심 코드 및 동작 분석](#153-python-가속기-제어-핵심-코드-및-동작-분석)
    - 15.4 [C++과 Python의 동일 하드웨어 트랜잭션 비교](#154-c과-python의-동일-하드웨어-트랜잭션-비교)
    - 15.5 [C++ 벤치마크 애플리케이션 실행 결과](#155-c-벤치마크-애플리케이션-실행-결과)
    - 15.6 [하드웨어 연산 결과 검증 로직](#156-하드웨어-연산-결과-검증-로직)
    - 15.7 [Python 기능 테스트 실행](#157-python-기능-테스트-실행)
    - 15.8 [성능 측정 체계](#158-성능-측정-체계)
    - 15.9 [CPU 연산 시간 측정 방식](#159-cpu-연산-시간-측정-방식)
    - 15.10 [FPGA 가속기 실행 시간 측정 방식](#1510-fpga-가속기-실행-시간-측정-방식)
    - 15.11 [성능 측정 요소 구분 (순수 연산 vs MMIO/드라이버 오버헤드 vs 데이터 전송)](#1511-성능-측정-요소-구분)
    - 15.12 [왜 CPU 기준선으로 Python을 사용하면 안 되는가?](#1512-왜-cpu-기준선으로-python을-사용하면-안-되는가)
    - 15.13 [폴링(Polling) vs 인터럽트 구동(Interrupt-Driven) 비교](#1513-폴링-vs-인터럽트-구동-비교)
16. [최종 검증 및 요약 (Final Validation &amp; Summary)](#16-최종-검증-및-요약)
  - [부록 A. PetaLinux에 추가 소프트웨어 패키지 추가 (Adding Extra Software Packages)](#부록-a-petalinux에-추가-소프트웨어-패키지-추가)
  - A.1 [패키지 설치 여부 사전 확인](#a1-패키지-설치-여부-사전-확인)
  - A.2 [루트 파일시스템 설정 메뉴 (petalinux-config -c rootfs)](#a2-루트-파일시스템-설정-메뉴)
  - A.3 [Python 3 패키지 활성화](#a3-python-3-패키지-활성화)
  - A.4 [GCC / G++ 패키지 활성화 (보드 내 네이티브 컴파일)](#a4-gcc--g-패키지-활성화)
  - A.5 [기타 유용한 개발 패키지 (git, make, cmake, vim, openssh 등)](#a5-기타-유용한-개발-패키지)
  - A.6 [선택된 패키지 설정 확인](#a6-선택된-패키지-설정-확인)
  - A.7 [루트 파일시스템 크기 및 메모리 최적화 고려사항](#a7-루트-파일시스템-크기-및-메모리-최적화-고려사항)

---

## 1. 튜토리얼 목표 및 시스템 아키텍처 (Tutorial Objective &amp; Architecture)

### 튜토리얼 목표

본 튜토리얼의 핵심 목표는 **AMD / Xilinx ZCU104 평가 보드(Zynq UltraScale+ MPSoC)** 상에서 FPGA 하드웨어 가속기를 **PetaLinux 기반 임베디드 리눅스 시스템**과 완벽하게 통합하고 유저스페이스(Userspace) 애플리케이션에서 제어하는 **재사용 가능한 범용 하드웨어/소프트웨어 통합 워크플로우(Hardware/Software Integration Workflow)**를 확립하는 것입니다.

본 튜토리얼에서 다루는 행렬 곱셈(Matrix Multiplication) 가속기는 전체 배포 프로세스를 실증하기 위한 **예제 워크로드(Example Workload)**일 뿐입니다. 여기서 확립한 워크플로우는 다음과 같은 다양한 HLS 또는 RTL 가속기 IP에 동일하게 적용 및 재사용할 수 있습니다:

- 메모리 복사 엔진 (Memory-Copy Engines)
- 영상/이미지 처리 커널 (Image-Processing Kernels)
- 합성곱 신경망(CNN) 추론 가속기 (CNN Accelerators)
- 유전체 시퀀스 정렬 가속기 (Sequence-Alignment Accelerators)
- 기타 커스텀 하드웨어 IP

### 시스템 레벨 전체 통합 워크플로우 파이프라인

```text
가속기 알고리즘 구현 (Accelerator Implementation)
       ↓
Vitis HLS / RTL IP 코어 생성 및 Export
       ↓
Vivado 하드웨어 통합 (Vivado Hardware Integration)
       ↓
AXI4-Lite 제어 경로 + AXI4 Master DDR 데이터 경로 구성
       ↓
하드웨어 플랫폼 파일 내보내기 (Export XSA with Bitstream)
       ↓
PetaLinux 하드웨어 정의 가져오기 (PetaLinux Hardware Import)
       ↓
디바이스 트리 자동 생성 및 커스텀 오버라이드 (Device-Tree Configuration)
       ↓
리눅스 UIO 드라이버 구성 (Linux Generic UIO Driver Configuration)
       ↓
CPU/FPGA 공유용 물리 DDR 메모리 예약 (Reserved DDR Memory Configuration)
       ↓
Linux 유저스페이스 애플리케이션 개발 (C++ / Python)
       ↓
가속기 MMIO 레지스터 설정 및 DMA 연산 실행 (Accelerator Execution)
       ↓
결과 무결성 검증 및 성능 측정 (Correctness & Performance Evaluation)
```

따라서 본 가이드의 궁극적인 목적은 행렬 곱셈 알고리즘 자체를 학습하는 것이 아니라, **Vivado/Vitis HLS에서 생성한 커스텀 FPGA 가속기를 PetaLinux 유저스페이스 프로그램에서 안정적으로 구동하는 전체 시스템 통합 기법을 마스터하는 것**입니다.

### 전체 시스템 아키텍처 개요 (Architecture Overview)

```text
+-----------------------------------------------------------------------------------+
|                            Linux Userspace Application                            |
|                          (C++: mmult-app / Python: mmult.py)                      |
|       +------------------------------------+-------------------------------+      |
|       | /dev/uio1 (mmult-control)          | /dev/uio0 (mmult-buffer)      |      |
|       | mmap() -> 64 KiB                   | mmap() -> 16 MiB              |      |
+-------+------------------------------------+-------------------------------+------+
        | (가상 메모리 매핑)                 | (가상 메모리 매핑)
        v                                    v
+-----------------------------------------------------------------------------------+
|                        Linux Kernel Space & Subsystems                            |
|  - UIO Platform Driver (uio_pdrv_genirq)                                          |
|  - Reserved Memory Subsystem (no-map, 0x70000000 - 0x70FFFFFF)                    |
|  - Device Tree (system.dtb: mmult@a0000000, mmult-buffer@70000000)               |
+-----------------------------------------------------------------------------------+
        |                                    |
        | AXI4-Lite 제어 신호                | Physical DDR 주소 전달
        v                                    v
+-----------------------------------------------------------------------------------+
|                  PS (Processing System: ARM Cortex-A53 APU)                       |
|  - M_AXI_HPM0_FPD (AXI Master to PL)                                              |
|  - S_AXI_HP0_FPD (AXI Slave from PL to DDR)                                       |
|  - PS DDR4 Controller (물리 메모리 0x70000000: Matrix A, B, C 버퍼)               |
+-----------------------------------------------------------------------------------+
        |                                    ^
        | AXI-Lite 트랜잭션                  | AXI4 Direct Memory Access
        v                                    |
+-----------------------------------------------------------------------------------+
|                     PL (Programmable Logic / FPGA Fabric)                         |
|  +-----------------------------------------------------------------------------+  |
|  |                 Matrix Multiplication Accelerator IP (HLS)                  |  |
|  |  [S_AXI_CONTROL: 0xA0000000]             [M_AXI_GMEM: AXI4 Master]          |  |
|  |   - AP_CTRL (START, DONE, IDLE)           - Direct Read Matrix A (DDR)      |  |
|  |   - Matrix A/B/C 물리 주소 레지스터       - Direct Read Matrix B (DDR)      |  |
|  |   - 행렬 차원 설정 (a_row, a_col, b_col)  - Direct Write Matrix C (DDR)     |  |
|  +-----------------------------------------------------------------------------+  |
+-----------------------------------------------------------------------------------+
```

### 본 튜토리얼을 통해 습득하는 11대 핵심 역량

1. 커스텀 가속기를 Zynq UltraScale+ PS(Processing System)에 통합하는 방법
2. 가속기 제어 레지스터(AXI-Lite)를 Linux 유저스페이스에 노출하는 방법
3. 가속기 AXI Master가 PS DDR 메모리에 직접 접근할 수 있도록 데이터 경로를 구성하는 방법
4. Vivado에서 생성된 XSA 하드웨어 플랫폼 파일을 PetaLinux 프로젝트로 임포트하는 방법
5. 생성된 디바이스 트리를 검증하고 `system-user.dtsi`를 통해 커스텀 오버라이드하는 방법
6. 유저스페이스 하드웨어 직접 접근을 위해 `generic-uio` 드라이버를 구성하는 방법
7. CPU와 FPGA 간 데이터 교환 시 리눅스 커널과의 충돌을 방지하기 위해 전용 물리 DDR 영역을 예약(Reserve)하는 방법
8. PetaLinux 유저스페이스 C++ 크로스 컴파일 애플리케이션 및 Python 스크립트를 빌드/배포하는 방법
9. PetaLinux 환경에서 FPGA 가속기를 기동(Start)하고 완료(Done) 상태를 모니터링하는 방법
10. FPGA 하드웨어 연산 결과를 ARM CPU 소프트웨어 참조 연산과 비교 검증(Validation)하는 방법
11. 복잡한 차세대 가속기 설계에 즉시 재사용할 수 있는 표준화된 개발 워크플로우 확립

---

## 2. PetaLinux 설치 및 ZCU104 BSP 환경 준비

가속기 하드웨어를 통합하기 전에 먼저 ZCU104 보드를 위한 안정적인 PetaLinux 기본 개발 환경을 구축해야 합니다.

### 2.1 필수 소프트웨어 및 파일 (Required Software &amp; Files)


| 구성 요소 (Component)        | 사용 버전 (Version)                                 | 사용 목적 (Purpose)                                   |
| :------------------------ | :----------------------------------------------- | :------------------------------------------------- |
| **Vivado Design Suite**  | 2024.1                                          | FPGA 블록 디자인, AXI 버스 통합, 비트스트림 생성, XSA 내보내기        |
| **Vitis HLS**            | 2024.1                                          | C/C++ 알고리즘으로부터 하드웨어 가속기 IP 생성 및 RTL 합성            |
| **PetaLinux Tools**      | 2024.1                                          | 임베디드 리눅스 커널, 디바이스 트리, rootfs, 부팅 이미지(BOOT.BIN) 빌드 |
| **ZCU104 PetaLinux BSP** | 2024.1 (`xilinx-zcu104-v2024.1-05230256.bsp`)   | ZCU104 보드 전용 초기 리눅스 보드 지원 패키지                     |
| **Ubuntu Linux**         | 24.04 LTS (Native 64-bit)                       | Vivado / Vitis HLS / PetaLinux를 모두 구동하는 단일 호스트 OS 환경 |
| **ZCU104 평가 보드**         | Rev C (Zynq UltraScale+ MPSoC)                  | 최종 하드웨어 배포 및 테스트 타깃 보드                            |


> [!IMPORTANT]
> **툴체인 버전 일치(Version Matching)의 중요성**  
> AMD/Xilinx 공식 문서에 따르면 PetaLinux 2024.1은 반드시 Vivado 2024.1에서 내보낸 XSA 및 2024.1 전용 BSP와 함께 사용해야 합니다.
>
> - `Vivado 2024.1 + PetaLinux 2023.2` ❌ (호환 불가)
> - `Vivado 2023.1 + PetaLinux 2024.1` ❌ (호환 불가)
> - `Vivado 2024.1 + PetaLinux 2024.1` ⭕ (정상 동작)

### 2.2 호스트 시스템 요구사항 (Host System Requirements)

- **RAM:** 최소 8 GB 이상 (16 GB 이상 권장)
- **CPU:** 2 GHz 이상 멀티코어 (8코어 이상 권장)
- **저장 공간 (Disk Space):** 최소 100 GB 이상의 여유 공간
- **호스트 OS:** 64-bit Linux (Ubuntu 24.04 LTS 및 22.04 LTS 지원)
- 본 튜토리얼에서는 **네이티브(Native) Ubuntu 24.04 LTS** 환경 한 대에서 Vivado, Vitis HLS, PetaLinux Tools를 모두 설치하여 진행하였습니다. (WSL이나 Windows 듀얼 부팅 없이 순수 Linux 머신에서 전체 워크플로우가 동작합니다.)

### 2.3 단일 네이티브 Linux 환경 구성

Vivado, Vitis HLS, PetaLinux를 전부 **하나의 네이티브 Ubuntu 머신**에 설치하여 사용하는 단일 환경 워크플로우입니다. Windows-WSL 조합처럼 OS를 오가며 파일을 복사할 필요가 없고, 모든 도구가 동일한 로컬 파일시스템을 공유합니다:

- **Vivado 2024.1 / Vitis HLS 2024.1:** 하드웨어 블록 디자인, `mmult_zcu104.xsa` 생성
- **PetaLinux 2024.1:** Yocto/BitBake, 리눅스 커널, 디바이스 트리, rootfs, `image.ub`, `BOOT.BIN` 빌드
- 두 도구가 생성하는 파일은 같은 사용자 홈 디렉토리 아래 로컬 경로(`cp` 한 줄)로 바로 주고받을 수 있습니다.

### 2.4 Ubuntu 사전 패키지 설치

PetaLinux 설치 전 필수 의존성 패키지를 설치합니다:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y gawk gcc make net-tools libssl-dev zlib1g-dev libncurses-dev diffstat texinfo chrpath socat cpio python3 rsync xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl2-dev pylint ctags build-essential libtinfo5 libtinfo6
```

### 2.5 로케일 설정 (Configure Locale)

PetaLinux/BitBake 빌드 시스템은 UTF-8 로케일을 필수로 요구합니다:

```bash
locale -a
sudo locale-gen en_US.UTF-8
sudo update-locale LANG=en_US.UTF-8
```

### 2.6 /bin/sh를 Bash로 변경

Ubuntu의 기본 셸인 Dash는 PetaLinux 빌드 스크립트와 호환되지 않으므로 반드시 Bash로 변경해야 합니다:

```bash
sudo dpkg-reconfigure dash
# 화면에 나타나는 질의에서 "No"를 선택합니다.
ls -l /bin/sh
# /bin/sh -> bash 로 링크되어 있는지 확인합니다.
```

### 2.7 PetaLinux 2024.1 인스톨러 다운로드

AMD 공식 다운로드 센터에서 `petalinux-v2024.1-final-installer.run` 파일을 다운로드합니다.

### 2.8 PetaLinux 2024.1 설치

설치 디렉토리를 생성하고 일반 사용자 계정으로 설치를 진행합니다:

```bash
mkdir -p ~/petalinux/2024.1
chmod +x petalinux-v2024.1-final-installer.run
./petalinux-v2024.1-final-installer.run --dir ~/petalinux/2024.1
```

> [!CAUTION]
> **절대 root(sudo) 권한으로 PetaLinux를 설치하거나 빌드하지 마십시오.**  
> BitBake 빌드 스크립트 오류 발생 시 시스템 파일이 훼손될 수 있으므로 일반 유저 권한으로만 실행해야 합니다.

### 2.9 설치 디렉토리 이동 금지 주의사항

PetaLinux 설치 디렉토리 경로에는 Yocto/eSDK의 절대 경로가 하드코딩되므로, 설치 후 다른 디렉토리로 이동하거나 복사해서는 안 됩니다.

### 2.10 PetaLinux 환경 변수 로드

새로운 터미널 세션을 열 때마다 PetaLinux 환경 변수를 로드합니다:

```bash
source ~/petalinux/2024.1/settings.sh
echo $PETALINUX
# 출력: /home/<user>/petalinux/2024.1
```

### 2.11 PetaLinux BSP 개요

BSP(Board Support Package)는 특정 개발 보드를 위해 사전 검증된 참조 플랫폼 설정 파일들을 포함하고 있습니다.

- 빈 프로젝트에서 시작하여 모든 ZynqMP 부품을 수동 설정하는 대신, 공식 ZCU104 BSP를 사용하면 정상 작동이 보증된 기본 리눅스 플랫폼에서 시작할 수 있습니다.

### 2.12 ZCU104 BSP 다운로드

AMD 공식 다운로드 센터의 임베디드 소프트웨어 페이지에서 ZCU104 전용 BSP인 `xilinx-zcu104-v2024.1-05230256.bsp` 파일을 다운로드합니다:

👉 **공식 다운로드 링크:** [AMD Embedded Software - ZCU104 BSP 다운로드 페이지](https://www.amd.com/en/support/downloads/adaptive-socs-and-fpgas/embedded-software.html#tabs-13aaba6de8-item-3e5066a8e9-tab)

### 2.13 BSP 파일 다운로드 위치 확인

네이티브 Ubuntu 환경이므로 브라우저로 다운로드한 파일이 곧바로 `~/Downloads/`에 저장됩니다. 별도의 OS 간 복사 과정 없이 파일 위치만 확인하면 됩니다:

```bash
ls -lh ~/Downloads/xilinx-zcu104-v2024.1-05230256.bsp
```

### 2.14 BSP 기반 PetaLinux 프로젝트 생성

```bash
mkdir -p ~/project
cd ~/project
source ~/petalinux/2024.1/settings.sh
petalinux-create -t project -s ~/Downloads/xilinx-zcu104-v2024.1-05230256.bsp
```

생성된 프로젝트 디렉토리: `~/project/xilinx-zcu104-2024.1/`

### 2.15 ZCU104 부팅 체인 및 PetaLinux 빌드 구성 요소

Zynq UltraScale+ MPSoC의 부팅 체인은 다음과 같습니다:

```text
cd 전원 투입 (Power-on / Reset)
       ↓
BootROM (온칩 ROM 코드 실행)
       ↓
FSBL (First Stage Boot Loader: zynqmp_fsbl.elf)
       ↓
PMU Firmware (전력 관리 장치 펌웨어: pmufw.elf)
       ↓
ARM Trusted Firmware (ATF: bl31.elf)
       ↓
U-Boot (u-boot.elf)
       ↓
Linux Kernel (Image) + Device Tree (system.dtb) + Root Filesystem (initramfs) -> [image.ub]
       ↓
Linux Userspace Applications
```

핵심 부팅 산출물 구분:

- `**BOOT.BIN**`: FSBL + PMU FW + FPGA Bitstream (`system.bit`) + ATF + U-Boot 통합 부팅 바이너리
- `**image.ub**`: 리눅스 커널 + 디바이스 트리 + rootfs (FIT 이미지 형태)
- `**boot.scr**`: U-Boot 자동 부팅 스크립트

### 2.16 순정(Unmodified) BSP 초기 빌드 검증

```bash
cd ~/project/xilinx-zcu104-2024.1
petalinux-build
```

빌드가 완료되면 `images/linux/` 디렉토리에 `Image`, `image.ub`, `system.dtb`, `system.bit`, `zynqmp_fsbl.elf`, `u-boot.elf` 등이 생성됩니다.

### 2.17 부팅 파일 생성 (BOOT.BIN)

```bash
petalinux-package --boot   --fsbl images/linux/zynqmp_fsbl.elf   --fpga images/linux/system.bit   --u-boot   --force
```

### 2.18 부팅 파일 SD 카드 복사 준비

네이티브 Ubuntu에서는 SD 카드 리더로 SD 카드를 직접 마운트하여 부팅 파일을 바로 복사합니다. 마운트 경로는 환경에 따라 `/media/$USER/BOOT` 등으로 다를 수 있으므로 `lsblk` 또는 파일 관리자로 실제 마운트 지점을 먼저 확인합니다:

```bash
lsblk
# 예: SD 카드 FAT32 파티션이 /media/$USER/BOOT 로 마운트된 경우
cp -f images/linux/BOOT.BIN images/linux/image.ub images/linux/boot.scr /media/$USER/BOOT/
```

### 2.19 SD 카드 파티션 및 파일 준비

SD 카드를 FAT32 포맷으로 포맷하고 다음 3개 파일을 루트에 복사합니다:

- `BOOT.BIN`
- `image.ub`
- `boot.scr`

### 2.20 ZCU104 SD 카드 부팅 모드 스위치 설정

ZCU104 보드의 부팅 모드 스위치 **SW6**을 SD1 부팅 모드로 설정합니다:


| 스위치 번호 | 1      | 2       | 3       | 4       |
| :------: | :------: | :-------: | :-------: | :-------: |
| **상태** | **ON** | **OFF** | **OFF** | **OFF** |


*(ZynqMP 모드 핀: `PS_MODE[3:0] = 1110`, ON은 논리 0, OFF는 논리 1)*

### 2.21 UART 시리얼 터미널 설정

PC와 ZCU104를 USB 케이블로 연결하고 시리얼 터미널(Tera Term, PuTTY 등)을 엽니다:

- **전송 속도(Baud Rate):** 115200
- **데이터:** 8비트, **패리티:** 없음, **정지 비트:** 1비트, **흐름 제어:** 없음

### 2.22 PetaLinux 부팅 검증

보드 전원을 켜고 터미널에서 리눅스 부팅을 확인합니다:

```bash
uname -a
# Linux ... 6.6.10-xilinx-v2024.1 ... aarch64 GNU/Linux
cat /proc/device-tree/model
# ZynqMP ZCU104 RevC
```

### 2.23 개발 중 수정 항목별 교체 대상 SD 카드 파일 매트릭스


| 수정한 항목 (Modification)             | SD 카드 교체 대상 파일                                  |
| :--------------------------------- | :----------------------------------------------- |
| **C++ 유저스페이스 애플리케이션**             | `image.ub`                                      |
| **Rootfs 패키지 추가/변경**              | `image.ub`                                      |
| **Linux 커널 설정 (`kernel config`)** | `image.ub`                                      |
| **UIO 드라이버 커널 설정**                | `image.ub`                                      |
| **디바이스 트리 (`system-user.dtsi`)**  | `image.ub`                                      |
| **커널 부트 아규먼트 (`bootargs`)**       | `image.ub`                                      |
| **FPGA 가속기 로직 / Vivado 블록 디자인**   | `**BOOT.BIN**` 및 `**image.ub**` (디바이스 트리 갱신 반영) |
| **새로운 XSA 하드웨어 플랫폼 임포트**          | `**BOOT.BIN**` 및 `**image.ub**`                 |
| **FSBL / PMU FW 변경**              | `BOOT.BIN`                                      |
| **U-Boot 부팅 설정 변경**               | `BOOT.BIN` (또는 `boot.scr`)                      |


### 2.24 권장 디렉토리 구조

```text
/home/<user>/
├── petalinux/
│   └── 2024.1/                     (PetaLinux 툴체인 설치 경로)
├── Downloads/
│   ├── petalinux-v2024.1-final-installer.run
│   └── xilinx-zcu104-v2024.1-05230256.bsp
└── project/
    ├── xilinx-zcu104-2024.1/       (PetaLinux 프로젝트 작업 공간)
    └── mmult_hw/                   (Vivado에서 내보낸 mmult_zcu104.xsa 보관)
```

---

## 3. 가속기 설계 참조 (Accelerator Design Reference)

본 튜토리얼은 가속기 하드웨어 설계 자체보다는 **PetaLinux 연동 워크플로우**에 집중합니다.  
선행 가이드인 *`Zybo Z7-10 기반 MemCopy 가속기 설계 및 구현 가이드`*에서 다룬 핵심 개념(AXI4 Master 메모리 매핑, AXI4-Lite 제어, HLS 합성, Vivado IP 패키징, 블록 디자인 통합, 인터럽트 연결 등)을 그대로 계승합니다.

일반적인 Vivado 호환 가속기 IP의 기본 구조:

- `**S_AXI_CONTROL` (AXI4-Lite Slave):** 프로세서의 가속기 제어 및 상태 모니터링
- `**M_AXI` (AXI4 Master):** 가속기가 직접 PS DDR 메모리를 고속으로 읽고 쓰기 위한 DMA 데이터 경로
- **클럭 및 리셋(Clock &amp; Reset):** `ap_clk`, `ap_rst_n`
- **Interrupt:** 가속기 연산 완료를 CPU GIC로 통보하는 `interrupt` 신호

---

## 4. 예제 가속기: 행렬 곱셈 (Example Accelerator: Matrix Multiplication)

본 튜토리얼에서는 16×16 크기의 시스톨릭 어레이(Systolic Array) 기반 행렬 곱셈 가속기($C = A 	imes B$)를 예제로 사용합니다.

- **연산 수식:** $C = A     imes B$
- **입력 인자:**
  - `a`: 입력 행렬 A 버퍼 주소 (64-bit)
  - `b`: 입력 행렬 B 버퍼 주소 (64-bit)
  - `c`: 출력 행렬 C 버퍼 주소 (64-bit)
  - `a_row`: 행렬 A의 행 수 (최대 16)
  - `a_col`: 행렬 A의 열 수 (최대 16)
  - `b_col`: 행렬 B의 열 수 (최대 16)

### 4.1 가속기 인터페이스 요구사항

- `**s_axi_control` (AXI4-Lite):** CPU가 가속기를 시작하고 상태를 확인하며 물리 버퍼 주소를 전달
- `**m_axi_gmem` (AXI4 Master):** 가속기가 PS DDR 메모리에서 행렬 A, B를 읽어오고 연산 결과 C를 DDR에 직접 기록

### 4.2 Vitis HLS IP 빌드 및 검증

1. Vitis HLS 프로젝트 생성 및 소스/테스트벤치 추가
2. C Simulation 실행 (PASS 확인)
3. C Synthesis 실행 (RTL 합성)
4. C/RTL Co-Simulation 실행 (PASS 확인)
5. Export RTL -&gt; Vivado IP 패키징 완료

---

## 5. AXI-Lite 레지스터 맵 분석 (AXI-Lite Register Map)

Vitis HLS 합성 후 생성된 `s_axi_control` 인터페이스의 레지스터 맵은 다음과 같습니다:


| 오프셋 (Offset) | 레지스터 이름        | 비트 폭 / 형식 | 설명 (Description)                           |
| :------------: | :--------------: | :---------: | :------------------------------------------ |
| `**0x00**`   | `**AP_CTRL**`  | 32-bit    | 제어 및 상태 레지스터 (Start / Done / Idle / Ready) |
| `**0x04**`   | `**GIE**`      | 32-bit    | Global Interrupt Enable 레지스터               |
| `**0x08**`   | `**IER**`      | 32-bit    | IP Interrupt Enable 레지스터                   |
| `**0x0C**`   | `**ISR**`      | 32-bit    | IP Interrupt Status 레지스터                   |
| `**0x10**`   | `**A[31:0]**`  | 32-bit    | 행렬 A 물리 주소 하위 32비트 (Data A Low)            |
| `**0x14**`   | `**A[63:32]**` | 32-bit    | 행렬 A 물리 주소 상위 32비트 (Data A High)           |
| `**0x1C**`   | `**B[31:0]**`  | 32-bit    | 행렬 B 물리 주소 하위 32비트 (Data B Low)            |
| `**0x20**`   | `**B[63:32]**` | 32-bit    | 행렬 B 물리 주소 상위 32비트 (Data B High)           |
| `**0x28**`   | `**C[31:0]**`  | 32-bit    | 행렬 C 물리 주소 하위 32비트 (Data C Low)            |
| `**0x2C**`   | `**C[63:32]**` | 32-bit    | 행렬 C 물리 주소 상위 32비트 (Data C High)           |
| `**0x34**`   | `**a_row**`    | 32-bit    | 행렬 A의 행 수 (Row count of A)                 |
| `**0x3C**`   | `**a_col**`    | 32-bit    | 행렬 A의 열 수 (Col count of A)                 |
| `**0x44**`   | `**b_col**`    | 32-bit    | 행렬 B의 열 수 (Col count of B)                 |


### AP_CTRL 비트 정의

- **Bit 0 (`AP_START`):** `1`을 쓰면 가속기 연산이 시작됩니다.
- **Bit 1 (`AP_DONE`):** 연산이 완료되면 가속기가 `1`로 세트합니다. (읽기 시 자동 클리어 또는 토글)
- **Bit 2 (`AP_IDLE`):** 가속기가 유휴 상태일 때 `1`입니다.
- **Bit 3 (`AP_READY`):** 다음 데이터를 수락할 준비가 되었을 때 `1`입니다.

> [!NOTE]
> Zynq UltraScale+는 64비트 아키텍처이므로 포인터 주소(A, B, C)가 64비트 레지스터(하위 32비트 + 상위 32비트) 쌍으로 분할되어 할당됩니다.

## 6. Vivado 하드웨어 통합 (Vivado Hardware Integration)

Vivado IP Integrator에서 Zynq UltraScale+ MPSoC PS와 HLS 가속기 IP를 다음과 같이 연결합니다:

### 6.1 제어 경로 (Control Path)

- **경로:** ARM Cortex-A53 (PS) -&gt; `M_AXI_HPM0_FPD` (Master) -&gt; AXI Interconnect -&gt; `mmult_0/s_axi_control` (Slave)
- **목적:** 리눅스 유저스페이스 애플리케이션이 가속기의 레지스터를 읽고 쓸 수 있도록 지원합니다.

### 6.2 데이터 경로 (Data Path)

- **경로:** `mmult_0/m_axi_gmem` (Master) -&gt; AXI SmartConnect -&gt; `S_AXI_HP0_FPD` (Slave) -&gt; PS DDR4 메모리
- **목적:** 가속기 하드웨어가 CPU의 개입 없이 PS DDR 메모리로부터 직접 입력 행렬 A, B를 읽어오고 결과 행렬 C를 직접 기록합니다.

### 6.3 인터럽트 경로 (Interrupt Path)

- **경로:** `mmult_0/interrupt` -&gt; `xlconcat` -&gt; `pl_ps_irq0` (PS) -&gt; ARM GIC (Generic Interrupt Controller)
- **목적:** 가속기 연산 완료 시 하드웨어 인터럽트를 발생시켜 CPU에 통보합니다. (초기 애플리케이션은 폴링 방식을 사용하더라도 하드웨어 결선을 미리 완료해 둡니다.)

### 6.4 클럭 및 리셋 (Clock &amp; Reset)

- **PL 클럭:** `pl_clk0` = 100 MHz
- **연결 대상:** `mmult_0/ap_clk`, AXI Interconnect 클럭, AXI SmartConnect 클럭, `proc_sys_reset_0`의 `slowest_sync_clk`

---

## 7. 주소 할당, 비트스트림 생성 및 XSA 내보내기 (Assign Address, Bitstream &amp; Export XSA)

### 7.1 Vivado Address Editor 설정

- `**mmult_0/s_axi_control`:** Base Address = `0xA000_0000`, Range = `64 KB` (주소 범위: `0xA000_0000` ~ `0xA000_FFFF`)
- `**mmult_0/m_axi_gmem`:** HP0 포트를 통해 PS DDR의 전체 주소 공간(`0x0000_0000` ~ `0x7FFF_FFFF` 등)에 접근할 수 있도록 자동 매핑

### 7.2 비트스트림 생성 및 XSA 내보내기

1. Vivado 메뉴에서 **Validate Design** 수행 (Critical Warning 및 Error 없음 확인)
2. Block Design에서 마우스 우클릭 -&gt; **Create HDL Wrapper** 선택
3. **Run Synthesis &amp; Implementation** 실행
4. **Generate Bitstream** 실행
5. 상단 메뉴: **File -&gt; Export -&gt; Export Hardware...** 선택
6. **Include bitstream** 옵션을 반드시 체크하고 내보내기 진행
7. 생성된 XSA 파일 예시: `~/vivado_projects/matmul_system/project_1/mmult_zcu104.xsa`

---

## 8. 하드웨어 설명 가져오기 및 PetaLinux 디바이스 트리 생성 확인

Vivado에서 생성한 `.xsa` 파일을 기존 ZCU104 PetaLinux 프로젝트로 임포트하여 하드웨어 변경 사항(AXI 주소, 인터럽트, 클럭 등)을 반영합니다.

### 8.1 XSA 복사 (Vivado 프로젝트 → PetaLinux 프로젝트)

Vivado와 PetaLinux가 같은 머신에 있으므로 로컬 경로 간 `cp` 한 줄로 끝납니다:

```bash
mkdir -p ~/project/mmult_hw
cp ~/vivado_projects/matmul_system/project_1/mmult_zcu104.xsa ~/project/mmult_hw/
ls -lh ~/project/mmult_hw/
```

### 8.2 PetaLinux 환경 로드 및 프로젝트 이동

```bash
source ~/petalinux/2024.1/settings.sh
cd ~/project/xilinx-zcu104-2024.1
```

### 8.3 Vivado 하드웨어 설명 임포트

```bash
petalinux-config --get-hw-description=~/project/mmult_hw/
```

이 명령을 실행하면 PetaLinux가 XSA 파일을 파싱하여 PS 설정, PL AXI 주소 맵, 가속기 정보, 인터럽트 설정 등을 업데이트하며 자동으로 메인 설정 메뉴가 실행됩니다.

### 8.4 DTG (Device Tree Generator) 설정 확인

설정 메뉴에서 다음 경로로 이동합니다:
`DTG Settings --->`

### 8.5 MACHINE_NAME 확인 ((AUTO) 설정 유지)

`**MACHINE_NAME**` 값이 **`(AUTO)`**로 되어 있는지 확인하고 **절대 임의로 변경하지 마십시오.**

- 본 프로젝트는 공식 ZCU104 BSP 기반으로 생성되었으므로, `(AUTO)`를 유지해야 BSP에 내장된 올바른 ZCU104 머신 설정과 보드 레벨 리눅스 구성이 그대로 유지됩니다.

### 8.6 MACHINE_NAME 설정의 중요성

`MACHINE_NAME`은 Yocto 빌드 시스템에 타깃 하드웨어 플랫폼을 식별시키는 핵심 항목으로, 머신별 커널 구성, 디바이스 트리 기본값, 보드 전용 패키지, 펌웨어 등에 영향을 미칩니다.

### 8.7 기타 DTG 옵션 점검

- `[ ] Devicetree overlay`: 비활성화 (부팅 시 비트스트림이 고정 로드되는 정적 디자인이므로 미사용)
- `[ ] Remove PL from devicetree`: **반드시 비활성화(체크 해제)** (PL 영역 가속기 노드가 디바이스 트리에 정상 포함되어야 함)
- `[ ] Disable alias generated by DTG`: 비활성화

### 8.8 설정 저장 및 종료

`<Save>`를 선택하고 `<Exit>`를 눌러 터미널로 복귀합니다.

### 8.9 머신 설정 검증

```bash
grep -i "MACHINE_NAME" project-spec/configs/config
```

### 8.10 디바이스 트리 독립 빌드

전체 시스템 빌드 전에 디바이스 트리가 정상 생성되는지 먼저 검증합니다:

```bash
petalinux-build -c device-tree
```

빌드 완료 후 `[INFO] Successfully built device-tree`가 출력되어야 합니다.

### 8.11 가속기 노드 자동 탐지 확인

```bash
grep -Rni -A15 -B3 "mmult" components/plnx_workspace/device-tree/
```

자동 생성된 디바이스 트리 노드 예시:

```dts
mmult_0: mmult@a0000000 {
    clock-names = "ap_clk";
    clocks = <&zynqmp_clk 71>;
    compatible = "xlnx,mmult-1.0";
    interrupt-names = "interrupt";
    interrupt-parent = <&gic>;
    interrupts = <0 89 4>;
    reg = <0x0 0xa0000000 0x0 0x10000>;
    xlnx,s-axi-control-addr-width = <0x7>;
    xlnx,s-axi-control-data-width = <0x20>;
};
```

- `reg = <0x0 0xa0000000 0x0 0x10000>`: Vivado에서 할당한 Base `0xA0000000`, Size `64 KB`와 정확히 일치함을 확인
- `interrupts = <0 89 4>`: Vivado에서 연결한 인터럽트 라인이 ARM GIC SPI 인터럽트(ID 89)로 정상 전파되었음을 확인

### 8.12 새로운 XSA 임포트 후 디바이스 트리 생성 실패 시 대처법

기존 BSP 프로젝트에 새로운 XSA를 임포트할 때 이전 빌드 캐시나 FSBL/PMU 잔여 파일로 인해 빌드 오류(`Failed to generate bsp for application zynqmp_fsbl`)가 발생할 수 있습니다.  
이 경우 다음 순서로 완전히 정리(clean) 후 재임포트합니다:

```bash
petalinux-build -x mrproper
petalinux-config --get-hw-description=~/project/mmult_hw/
# DTG Settings 확인 후 저장
petalinux-build -c device-tree
```

---

## 9. UIO 접근 구성 및 가속기용 DDR 메모리 예약

디바이스 트리가 생성되었으면, 리눅스 유저스페이스에서 하드웨어를 직접 제어할 수 있도록 **두 개의 독립된 메모리 영역**을 UIO로 노출해야 합니다:

1. **가속기 제어 레지스터 (Accelerator Control Registers):** AXI-Lite 레지스터 공간 (`0xA0000000`, 64 KiB)
2. **가속기 데이터 버퍼 (Accelerator Data Buffers):** CPU/FPGA 공유 물리 DDR 메모리 (`0x70000000`, 16 MiB)

### 9.1 가속기 제어 주소 영역

가속기 제어 인터페이스는 `0xA0000000` ~ `0xA000FFFF` 주소 공간에 위치하며 `AP_CTRL`, 주소 포인터, 행렬 차원 등의 레지스터를 포함합니다.

### 9.2 UIO를 통한 가속기 제어 레지스터 노출

기본 생성된 가속기 노드의 `compatible = "xlnx,mmult-1.0";`을 **`generic-uio`**로 오버라이드하여 전용 커널 드라이버 작성 없이 유저스페이스에서 `/dev/uioX`로 제어할 수 있도록 합니다.

수정 파일: `project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi`

```dts
&mmult_0 {
    compatible = "generic-uio";
    linux,uio-name = "mmult-control";
};
```

- `&mmult_0`: DTG가 자동 생성한 노드의 라벨을 참조하여 속성만 오버라이드합니다.
- `linux,uio-name = "mmult-control"`: 유저스페이스 프로그램이 `/sys/class/uio/uioX/name`을 통해 디바이스를 안전하게 식별할 수 있도록 이름을 지정합니다.

### 9.3 DDR 메모리 예약(Reserved Memory)이 필수적인 이유

가속기는 연산 시 고정된 물리 DDR 주소(`0x70000000`)에 직접 DMA 접근(M_AXI)을 수행합니다.  
만약 이 물리 메모리 영역을 리눅스 커널로부터 예약(Reserve)하지 않으면, 리눅스 커널의 메모리 관리자(Buddy Allocator / SLUB)가 해당 영역을 일반 프로세스, 힙, 스택, 페이지 캐시 등으로 임의 할당할 수 있습니다.  
이 경우 FPGA가 데이터를 쓰는 순간 **커널 메모리 오염, 데이터 훼손, 시스템 패닉 및 크래시**가 발생합니다. 따라서 해당 물리 DDR 영역을 리눅스 일반 메모리 풀에서 완전히 격리해야 합니다.

### 9.4 DDR 영역 예약 설정 (reserved-memory, no-map)

- 시작 주소: `0x70000000`
- 크기: `0x01000000` (16 MiB)
- `no-map` 속성: 리눅스 커널이 이 영역을 자신의 가상 주소 공간(Direct Mapping)에 매핑하지 않도록 지시합니다.

### 9.5 예약된 DDR 영역을 UIO로 노출 (mmult-buffer)

유저스페이스 애플리케이션이 예약된 물리 DDR 영역을 `mmap()`하여 입출력 행렬 데이터를 읽고 쓸 수 있도록 별도의 UIO 노드를 선언합니다.

### 9.6 왜 malloc()을 가속기 버퍼로 직접 사용할 수 없는가?

일반 리눅스 애플리케이션에서 `malloc()`으로 할당받은 포인터는 **가상 주소(Virtual Address)**입니다.

- CPU는 ARM MMU(Memory Management Unit)를 통해 가상 주소를 물리 주소로 변환하여 접근합니다.
- 그러나 FPGA 가속기는 MMU를 거치지 않고 AXI 버스 상에 **물리 주소(Physical Address)**를 직접 전송합니다.
- 또한 `malloc()`으로 할당된 가상 메모리는 연속되어 보이지만 물리 메모리 상에서는 불연속적인 여러 4KB 페이지로 쪼개져 있을 수 있습니다.
- 따라서 고정된 연속 물리 DDR 영역을 예약하고 이를 사용하는 것이 가장 안정적이고 직관적인 방식입니다.

### 9.7 행렬 버퍼 레이아웃 (Matrix Buffer Layout)

예약된 16 MiB 공간(`0x70000000` ~ `0x70FFFFFF`) 내에서 각 행렬 버퍼를 4 KiB 페이지 경계에 맞추어 배치합니다:

```text
예약된 물리 DDR (16 MiB)
0x70000000  +-------------------------------------+  오프셋 0x0000 (Matrix A: 16x16x4B = 1 KiB)
            |              Matrix A               |
0x70001000  +-------------------------------------+  오프셋 0x1000 (Matrix B: 16x16x4B = 1 KiB)
            |              Matrix B               |
0x70002000  +-------------------------------------+  오프셋 0x2000 (Matrix C: 16x16x4B = 1 KiB)
            |              Matrix C               |
0x70003000  +-------------------------------------+  미사용 예약 공간 (추후 확장 가능)
            |                 ...                 |
0x70FFFFFF  +-------------------------------------+
```

### 9.8 완전한 system-user.dtsi 소스 코드 및 3대 핵심 기능

`project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi` 파일의 전체 내용입니다:

```dts
/include/ "system-conf.dtsi"
/ {
    reserved-memory {
        #address-cells = <2>;
        #size-cells = <2>;
        ranges;

        mmult_reserved: buffer@70000000 {
            no-map;
            reg = <0x0 0x70000000 0x0 0x01000000>;
        };
    };

    mmult_buffer: mmult-buffer@70000000 {
        compatible = "generic-uio";
        linux,uio-name = "mmult-buffer";
        reg = <0x0 0x70000000 0x0 0x01000000>;
    };
};

&mmult_0 {
    compatible = "generic-uio";
    linux,uio-name = "mmult-control";
};
```

**3대 핵심 기능 요약:**

1. **DDR 예약 (`buffer@70000000`):** `0x70000000`부터 16 MiB 영역을 리눅스 기본 메모리 할당 풀에서 제외
2. **데이터 UIO 노드 노출 (`mmult-buffer`):** 예약된 16 MiB DDR을 유저스페이스에서 `mmap()`할 수 있도록 UIO 디바이스로 등록
3. **제어 UIO 노드 오버라이드 (`mmult-control`):** `0xA0000000` 가속기 AXI-Lite 레지스터를 UIO 디바이스로 등록

### 9.9 제어 공간(Control Space) vs 데이터 공간(Data Space) 비교


| 항목                        | 제어 공간 (`mmult-control`)               | 데이터 공간 (`mmult-buffer`)                  |
| :------------------------- | :------------------------------------- | :---------------------------------------- |
| **물리 주소 (Physical Addr)** | `0xA000_0000`                         | `0x7000_0000`                            |
| **메모리 크기 (Size)**         | 64 KiB (`0x10000`)                    | 16 MiB (`0x01000000`)                    |
| **하드웨어 인터페이스**            | AXI4-Lite Slave (`s_axi_control`)     | PS DDR4 DRAM                             |
| **주요 역할**                 | 가속기 레지스터 설정, 실행(START), 상태 모니터링(DONE) | 행렬 A, B 입력 데이터 기록 및 연산 결과 C 수신           |
| **CPU 접근 방식**             | `/dev/uioX` `mmap()` 후 레지스터 RW        | `/dev/uioY` `mmap()` 후 오프셋 기반 버퍼 RW      |
| **FPGA 접근 방식**            | CPU가 보내는 AXI-Lite 신호 수신               | 가속기 Master(M_AXI)가 AXI Burst로 직접 메모리 R/W |


### 9.10 고정 예약 DDR 방식이 항상 필수적인가?

- 본 튜토리얼에서 사용한 **고정 물리 주소 예약 + UIO 매핑** 방식은 구조가 매우 투명하고 디버깅이 쉬워 **초기 하드웨어 검증, 프로토타입 개발, 교육용**으로 최적입니다.
- 상용 프로덕션 임베디드 리눅스 시스템에서는 고정 주소 대신 다음과 같은 동적 메모리 관리 기법을 주로 활용합니다:
  - **DMA Kernel Driver:** `dma_alloc_coherent()`를 통한 동적 물리 연속 버퍼 할당
  - **CMA (Contiguous Memory Allocator):** 리눅스 커널 CMA 풀을 이용한 동적 연속 메모리 할당
  - **DMA-BUF / `udmabuf`:** 유저스페이스와 하드웨어 드라이버 간 DMA 버퍼 공유 프레임워크

### 9.11 디바이스 트리 빌드 및 역컴파일(dtc)을 통한 검증

```bash
petalinux-build -c device-tree -x cleansstate
petalinux-build -c device-tree

# dtc 도구 설치 (필요 시)
sudo apt update && sudo apt install -y device-tree-compiler

# 최종 system.dtb를 dts로 역컴파일하여 검증
dtc -I dtb -O dts -o /tmp/mmult-final.dts images/linux/system.dtb

# 핵심 노드 검색
grep -n -A15 -B5 "mmult-control\|mmult-buffer\|70000000\|a0000000" /tmp/mmult-final.dts
```

역컴파일된 DTS에서 `mmult-control`, `mmult-buffer`, `buffer@70000000` 노드가 정상 반영되었는지 확인합니다.

## 10. PetaLinux에서 Generic UIO 지원 활성화 (Enable Generic UIO Support)

디바이스 트리에 `compatible = "generic-uio";`를 선언했더라도, 리눅스 커널 내에 Generic UIO 드라이버(`uio_pdrv_genirq`)가 내장(Built-in)되어 있어야 하며 부트 파라미터가 설정되어 있어야 실제 `/dev/uioX` 디바이스 노드가 생성됩니다.

### 10.1 리눅스 커널에서 UIO 활성화

커널 설정 메뉴를 실행합니다:

```bash
petalinux-config -c kernel
```

메뉴 탐색:
`Device Drivers --->`  
  `<*> Userspace I/O drivers --->`  
    `<*> Userspace I/O platform driver with generic IRQ handling`

- 반드시 모듈(`M`)이 아닌 **빌트인(`*` 또는 `y`)**으로 활성화해야 합니다.
- 저장 후 종료합니다.

### 10.2 최종 UIO 커널 설정 검증 및 bsp.cfg 오버라이드

커널 빌드 후 최종 `.config` 파일을 확인합니다:

```bash
petalinux-build -c kernel
grep -E '^CONFIG_UIO=|^CONFIG_UIO_PDRV_GENIRQ=' build/tmp/work-shared/xilinx-zcu104/kernel-build-artifacts/.config
```

정상 출력:

```text
CONFIG_UIO=y
CONFIG_UIO_PDRV_GENIRQ=y
```

> [!WARNING]
> **OpenAMP 설정과의 병합 충돌 방지 (`bsp.cfg` 오버라이드)**  
> 만약 출력 결과가 `CONFIG_UIO_PDRV_GENIRQ=m` (모듈)로 되어 있다면, OpenAMP 등 다른 커널 설정 단편(Fragment)이 빌트인 설정을 덮어쓴 것입니다.  
> 이 경우 `project-spec/meta-user/recipes-kernel/linux/linux-xlnx/bsp.cfg` 파일을 열어 다음을 추가합니다:
>
> ```text
> CONFIG_UIO=y
> CONFIG_UIO_PDRV_GENIRQ=y
> ```
>
> 이후 커널을 재빌드합니다:
>
> ```bash
> petalinux-build -c kernel -x cleansstate
> petalinux-build -c kernel
> ```

### 10.3 커널 부트 Argument에 Generic UIO 매핑 추가

리눅스 커널이 부팅 시 `generic-uio` compatible 문자열을 가진 디바이스 트리 노드를 `uio_pdrv_genirq` 드라이버에 자동으로 바인딩하도록 커널 부트 argument를 설정합니다.

```bash
petalinux-config
```

메뉴 탐색:
`DTG Settings --->`  
  `Kernel Bootargs --->`  
    `[ ] generate boot args automatically`  (자동 생성 옵션 체크 해제)

`User Set Kernel Bootargs` 항목에 다음 문자열을 입력합니다:

```text
earlycon console=ttyPS0,115200 clk_ignore_unused init_fatal_sh=1 uio_pdrv_genirq.of_id=generic-uio
```

- 핵심 파라미터: `**uio_pdrv_genirq.of_id=generic-uio**`
- 이 인자가 전달되면 커널이 `compatible = "generic-uio"`인 모든 노드(`mmult-control`, `mmult-buffer`)를 해당 드라이버에 즉시 연결합니다.

### 10.4 최종 DTB 역컴파일을 통한 부트 아규먼트 검증

```bash
petalinux-build -c device-tree
dtc -I dtb -O dts -o /tmp/mmult-final.dts images/linux/system.dtb
grep -n "bootargs" /tmp/mmult-final.dts
```

출력 결과의 `chosen { bootargs = "..."; };` 항목에 `uio_pdrv_genirq.of_id=generic-uio`가 포함되어 있는지 확인합니다.

---

## 11. PetaLinux 루트 파일시스템 준비 (Prepare Root Filesystem)

### 11.1 petalinux-image-minimal 설정

기본 초소형 initramfs 대신 커스텀 애플리케이션 및 유틸리티가 정상 포함될 수 있도록 루트 이미지 패키징을 `petalinux-image-minimal`로 변경합니다.

```bash
petalinux-config
```

메뉴 탐색:
`Image Packaging Configuration --->`  
  `INITRAMFS/INITRD Image name` -&gt; `**petalinux-image-minimal**` 입력 후 저장

### 11.2 개발 편의를 위한 자동 Root 로그인(auto-login) 활성화

반복적인 하드웨어 테스트 및 디버깅 시 매번 로그인 프롬프트(`xilinx-zcu104-20241 login:`)를 거치지 않고 곧바로 root 셸로 진입할 수 있도록 자동 로그인을 설정합니다.

```bash
petalinux-config -c rootfs
```

메뉴 탐색:
`Image Features --->`  
  `[*] auto-login` (체크 활성화)

---

## 12. Linux 유저스페이스 애플리케이션: C++ 및 Python

PetaLinux 시스템이 `mmult-control` 및 `mmult-buffer` UIO 디바이스를 제공하면 유저스페이스에서 C++ 또는 Python으로 가속기를 완전히 제어할 수 있습니다.

### 12.1 공통 소프트웨어-하드웨어 인터페이스 및 UIO 탐색 흐름

두 애플리케이션 모두 하드코딩된 디바이스 경로(`/dev/uio0`, `/dev/uio1`) 대신 `/sys/class/uio/uioX/name`의 문자열(`mmult-control`, `mmult-buffer`)을 동적으로 검색하여 디바이스 번호 변동에 안전하게 대응합니다.

```text
[ 유저스페이스 애플리케이션 공통 실행 흐름 ]
1. /sys/class/uio/uio*/name 검색 -> mmult-control 디바이스 탐색
2. /sys/class/uio/uio*/name 검색 -> mmult-buffer 디바이스 탐색
3. 해당 /dev/uioX 및 /dev/uioY 파일 open()
4. mmap()을 통한 64 KiB 가속기 제어 레지스터 매핑
5. mmap()을 통한 16 MiB 예약 DDR 버퍼 매핑
6. 매핑된 DDR 버퍼에 행렬 A, B 초기 데이터 기록
7. 가속기 레지스터에 물리 주소(A=0x70000000, B=0x70001000, C=0x70002000) 프로그램
8. 가속기 레지스터에 행렬 크기(a_row=16, a_col=16, b_col=16) 프로그램
9. AP_START 비트 세트 (0x00 레지스터에 0x01 기록)
10. AP_DONE 비트 폴링 대기 (0x00 레지스터의 bit 1 모니터링)
11. 매핑된 DDR 버퍼(오프셋 0x2000)에서 결과 행렬 C 판독
12. ARM CPU 소프트웨어 연산 결과와 FPGA 하드웨어 연산 결과 1:1 비교 검증
```

### 12.2 C++ 애플리케이션 레시피 생성 및 활성화

```bash
cd ~/project/xilinx-zcu104-2024.1
petalinux-create -t apps --template c++ -n mmult-app --enable
```

생성된 디렉토리: `project-spec/meta-user/recipes-apps/mmult-app/`  
해당 디렉토리 내의 `files/mmult-app.cpp` 파일을 프로젝트 GitHub 저장소에서 제공하는 소스 코드로 교체합니다.

설정 활성화 확인:

```bash
grep -Rni "mmult-app" project-spec/configs/rootfs_config
# CONFIG_mmult-app=y 확인
```

### 12.3 C++ 애플리케이션 빌드

```bash
petalinux-build -c mmult-app
```

PetaLinux 크로스 툴체인을 통해 ARM Cortex-A53 (AArch64) 타깃용 바이너리가 컴파일됩니다.

### 12.4 호스트에서 C++ 크로스 컴파일을 수행하는 이유

- 개발 PC 호스트(x86-64)와 ZCU104 타깃 보드(ARM AArch64)의 CPU 아키텍처가 완전히 다릅니다.
- 호스트의 일반 `g++`로 빌드하면 x86-64 바이너리가 생성되어 타깃에서 실행되지 않으므로, PetaLinux 크로스 컴파일 툴체인을 통해 AArch64 바이너리를 생성하고 rootfs에 통합합니다.

### 12.5 Python 애플리케이션 개요

- Python 스크립트(`mmult.py`)는 인터프리터 언어이므로 별도의 사전 크로스 컴파일 없이 보드 상의 Python 3 인터프리터에서 즉시 실행할 수 있어 **빠른 하드웨어 디버깅 및 시제품 테스트**에 매우 유리합니다.

### 12.6 Python 스크립트를 보드로 전달하는 방법

1. **SCP 네트워크 전송:**
  ```bash
   scp mmult.py root@[[ORCA_RICH_MD:b23598064449112d2ef41321e2de99a1:inline-html:%3CZCU104_IP%3E]]:/root/
  ```
2. **SD 카드 FAT32 파티션을 통한 전달:**
  - SD 카드의 첫 번째 파티션에 `mmult.py` 복사
  - 보드 상에서 마운트 후 실행:
    ```bash
    mkdir -p /mnt/sd
    mount /dev/mmcblk0p1 /mnt/sd
    python3 /mnt/sd/mmult.py
    ```

### 12.7 C++과 Python의 동일 메모리 맵 접근 구조

```text
[ Linux Userspace ]
    C++ Application (mmult-app)           Python Application (mmult.py)
            | (mmap)                               | (mmap)
            +-------------------+------------------+
                                |
                                v
[ Linux UIO Framework ]
        /dev/uioX (mmult-control)          /dev/uioY (mmult-buffer)
                |                                  |
                v                                  v
        물리 주소 0xA0000000               물리 주소 0x70000000
        (AXI-Lite 제어 레지스터)           (공유 PS DDR 메모리)
                |                                  ^
                | AXI-Lite 트랜잭션                | AXI4 Direct DMA
                v                                  |
    +-------------------------------------------------------------+
    |                    FPGA Accelerator IP                      |
    +-------------------------------------------------------------+
```

### 12.8 C++ vs Python 종합 비교


| 비교 항목 (Aspect)      | C++ (`mmult-app.cpp`)          | Python (`mmult.py`)        |
| :------------------- | :------------------------------ | :-------------------------- |
| **사전 컴파일 필요 여부**    | 필요 (AArch64 크로스 컴파일)           | 불필요 (인터프리터 직접 실행)          |
| **PetaLinux 빌드 통합** | `petalinux-create -t apps` 레시피 | 스크립트 파일 단순 복사              |
| **수정 후 반영 속도**      | 빌드 및 배포 주기 필요                  | 코드 수정 후 즉시 재실행 가능          |
| **UIO 및 DDR 접근 능력** | `open()`, `mmap()` 완벽 지원       | `mmap`, `struct` 모듈로 완벽 지원 |
| **CPU 연산 벤치마크 기준선** | **적합 (네이티브 ARM 기계어 실행)**       | **부적합 (인터프리터 오버헤드 왜곡)**    |
| **주요 사용 목적**        | **최종 상용 배포 및 정밀 성능 평가**        | **초기 하드웨어 브링업 및 빠른 디버깅**   |


---

## 13. 최종 시스템 빌드, 패키징 및 SD 카드 부팅 준비

모든 하드웨어 임포트, 디바이스 트리 커스텀, UIO 커널 설정, rootfs 구성, 유저스페이스 애플리케이션 작성이 완료되었으므로 최종 시스템을 빌드하고 부팅 이미지를 생성합니다.

### 13.1 전체 PetaLinux 프로젝트 빌드

```bash
source ~/petalinux/2024.1/settings.sh
cd ~/project/xilinx-zcu104-2024.1
petalinux-build
```

빌드 성공 시 `[INFO] Successfully built project` 메시지와 함께 `images/linux/` 디렉토리에 모든 산출물이 생성됩니다.

### 13.2 BOOT.BIN 생성

```bash
petalinux-package --boot   --fsbl images/linux/zynqmp_fsbl.elf   --fpga images/linux/system.bit   --u-boot   --force
```

### 13.3 petalinux-package --boot 실행 필요 여부 판단 기준

모든 수정 작업마다 `petalinux-package --boot`를 다시 실행할 필요는 없습니다.

- `**image.ub`만 교체하면 되는 경우 (Linux 소프트웨어 영역 수정):**
  - C++ 유저스페이스 애플리케이션 코드 수정
  - `system-user.dtsi` 디바이스 트리 수정
  - 커널 설정 및 UIO 설정 변경
  - 커널 부트 아규먼트(`bootargs`) 변경
  - Rootfs 패키지 추가
- `**BOOT.BIN`을 반드시 재생성하고 교체해야 하는 경우 (하드웨어/초기 부팅 펌웨어 수정):**
  - Vivado 블록 디자인 및 가속기 RTL/HLS 로직 수정
  - 새로운 비트스트림(`system.bit`) 생성 및 새로운 XSA 임포트
  - FSBL(`zynqmp_fsbl.elf`) 또는 PMU 펌웨어 변경
  - U-Boot 자체 설정 변경

### 13.4 예제 1: 소프트웨어 전용 수정 시 업데이트 워크플로우

`mmult-app.cpp` 또는 `system-user.dtsi`만 수정한 경우:

```bash
petalinux-build
cp -f images/linux/image.ub /media/$USER/BOOT/
```

-&gt; SD 카드의 `image.ub` 파일만 교체 후 재부팅

### 13.5 예제 2: 하드웨어 수정 시 업데이트 워크플로우

Vivado에서 가속기 로직 변경 후 새 XSA를 내보낸 경우:

```bash
petalinux-config --get-hw-description=~/project/mmult_hw/
petalinux-build
petalinux-package --boot --fsbl images/linux/zynqmp_fsbl.elf --fpga images/linux/system.bit --u-boot --force
cp -f images/linux/BOOT.BIN images/linux/image.ub /media/$USER/BOOT/
```

-&gt; SD 카드의 `BOOT.BIN`과 `image.ub`를 모두 교체 후 재부팅

### 13.6 부팅 파일 SD 카드 복사

네이티브 Ubuntu에서 SD 카드를 직접 마운트하여 복사합니다 (마운트 경로는 `lsblk`로 확인, 환경에 따라 다를 수 있음):

```bash
lsblk
cp -f images/linux/BOOT.BIN images/linux/image.ub images/linux/boot.scr /media/$USER/BOOT/
```

### 13.7 타깃 보드 부팅 및 콘솔 확인

SD 카드를 ZCU104 보드에 삽입하고 전원을 켜면 U-Boot 및 리눅스 부팅 과정을 거쳐 자동 로그인 프롬프트로 진입합니다:

```text
root@xilinx-zcu104-20241:~#
```

## 14. ZCU104 보드 상에서 UIO 디바이스 및 메모리 매핑 검증

가속기 애플리케이션을 실행하기 전에 보드 상에서 UIO 디바이스가 정상 인식되고 메모리 매핑이 올바르게 구성되었는지 반드시 사전 검증을 수행합니다.

### 14.1 실행 중인 리눅스 시스템 확인

UART 시리얼 콘솔에서 보드가 정상 부팅되었는지 확인합니다.

### 14.2 UIO 디바이스 노드 확인

```bash
ls -l /dev/uio*
```

- ZCU104 BSP에는 AXI Performance Monitor(pmon) 등 다른 UIO 디바이스가 이미 존재할 수 있으므로 `/dev/uio0`부터 여러 개가 나열될 수 있습니다.

### 14.3 이름 기반 UIO 디바이스 식별 스크립트

각 UIO 번호에 매핑된 실제 하드웨어 이름을 확인합니다:

```bash
for d in /sys/class/uio/uio*; do
    echo "$d : $(cat $d/name)"
done
```

출력 예시:

```text
/sys/class/uio/uio0 : mmult-buffer
/sys/class/uio/uio1 : mmult-control
/sys/class/uio/uio2 : axi-pmon
/sys/class/uio/uio3 : axi-pmon
...
```

- 반드시 `mmult-buffer`와 `mmult-control`이 목록에 존재해야 합니다.

### 14.4 Generic UIO 드라이버 바인딩 확인

```bash
ls -l /sys/bus/platform/drivers/uio_pdrv_genirq/
```

출력 항목에 `a0000000.mmult` 및 `70000000.mmult-buffer`가 디렉토리로 존재해야 드라이버가 정상 바인딩된 것입니다.

### 14.5 각 UIO 매핑의 물리 주소 및 크기 확인

각 UIO 디바이스의 물리 메모리 매핑 정보(`maps/map0/addr`, `maps/map0/size`)를 확인합니다:

```bash
# 버퍼 UIO (예: uio0)
cat /sys/class/uio/uio0/maps/map0/addr   # 0x0000000070000000
cat /sys/class/uio/uio0/maps/map0/size   # 0x0000000001000000 (16 MiB)

# 제어 UIO (예: uio1)
cat /sys/class/uio/uio1/maps/map0/addr   # 0x00000000a0000000
cat /sys/class/uio/uio1/maps/map0/size   # 0x0000000000010000 (64 KiB)
```

### 14.6 모든 UIO 매핑 일괄 검증 원라이너 스크립트

```bash
for d in /sys/class/uio/uio*; do
    echo "=============================="
    echo "Device : $(basename $d)"
    echo "Name   : $(cat $d/name)"
    if [ -f "$d/maps/map0/addr" ]; then
        echo "Addr   : $(cat $d/maps/map0/addr)"
        echo "Size   : $(cat $d/maps/map0/size)"
    fi
done
```

### 14.7 런타임 디바이스 트리 노드(of_node) 검증

```bash
# 플랫폼 디바이스 확인
ls /sys/bus/platform/devices/ | grep -E "mmult|a0000000|70000000"

# compatible 및 uio-name 속성 확인
tr '\0' '\n' < /sys/bus/platform/devices/a0000000.mmult/of_node/compatible
# generic-uio
tr '\0' '\n' < /sys/bus/platform/devices/a0000000.mmult/of_node/linux,uio-name
# mmult-control

tr '\0' '\n' < /sys/bus/platform/devices/70000000.mmult-buffer/of_node/compatible
# generic-uio
tr '\0' '\n' < /sys/bus/platform/devices/70000000.mmult-buffer/of_node/linux,uio-name
# mmult-buffer
```

### 14.8 예약된 DDR 레이아웃 및 가상-물리 주소 매핑 검증


| 행렬 (Matrix)  | Linux 유저스페이스 `mmap` 오프셋 | FPGA 가속기 물리 주소 (Physical Addr) | 데이터 크기           |
| :------------: | :-----------------------: | :------------------------------: | :----------------: |
| **행렬 A** | `+0x0000`               | `0x7000_0000`                  | 1 KiB (16×16×4B) |
| **행렬 B** | `+0x1000`               | `0x7000_1000`                  | 1 KiB (16×16×4B) |
| **행렬 C** | `+0x2000`               | `0x7000_2000`                  | 1 KiB (16×16×4B) |


### 14.9 C++ 실행 바이너리 확인

```bash
which mmult-app
# /usr/bin/mmult-app
```

### 14.10 장애 진단 및 문제 해결 (Troubleshooting Guide)

- **증상 1: `/dev/uio*` 디바이스 노드가 전혀 나타나지 않음**
  - 원인: 커널 부트 아규먼트 누락 또는 커널 UIO 드라이버 미활성화
  - 조치: `cat /proc/cmdline`에 `uio_pdrv_genirq.of_id=generic-uio`가 포함되어 있는지 확인하고, 커널 `.config`의 `CONFIG_UIO=y` 및 `CONFIG_UIO_PDRV_GENIRQ=y` 확인
- **증상 2: `uio_pdrv_genirq` 드라이버는 있으나 `mmult` 디바이스가 바인딩되지 않음**
  - 원인: 디바이스 트리(`system-user.dtsi`)에 `compatible = "generic-uio";` 속성 오버라이드 누락
  - 조치: `dtc`로 `system.dtb`를 역컴파일하여 `mmult@a0000000` 및 `mmult-buffer@70000000`의 compatible 속성 확인

---

## 15. 가속기 실행, 검증 및 성능 측정

### 15.1 유저스페이스 가속기 호출 8단계 시퀀스

1. `/sys/class/uio/uio*/name`에서 `mmult-control` 및 `mmult-buffer` 디바이스 노드 검색
2. 두 UIO 디바이스 파일 open 후 `mmap()`으로 가상 주소 공간에 매핑
3. 매핑된 DDR 버퍼에 입력 행렬 A, B 데이터 기록
4. 가속기 AXI-Lite 레지스터에 물리 주소(A, B, C) 기록
5. 가속기 레지스터에 행렬 크기(a_row, a_col, b_col) 기록
6. `AP_CTRL` 레지스터의 `AP_START` 비트(Bit 0)를 `1`로 세트하여 가속기 구동
7. `AP_CTRL` 레지스터의 `AP_DONE` 비트(Bit 1)가 `1`이 될 때까지 폴링 대기
8. 매핑된 DDR 버퍼(오프셋 0x2000)에서 연산 결과 행렬 C 판독 및 검증

### 15.2 C++ 가속기 제어 핵심 코드 및 동작 분석

```cpp
// 1. UIO 제어 레지스터 매핑 (64 KiB)
int ctrl_fd = open("/dev/uio1", O_RDWR);
volatile uint32_t *regs = reinterpret_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cvolatile%20uint32_t%20*%3E]](
    mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, ctrl_fd, 0));

// 2. UIO 예약 DDR 버퍼 매핑 (16 MiB)
int buffer_fd = open("/dev/uio0", O_RDWR);
void *buffer = mmap(nullptr, 0x01000000, PROT_READ | PROT_WRITE, MAP_SHARED, buffer_fd, 0);

// 3. 오프셋 기반 포인터 설정
int *A = reinterpret_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cint%20*%3E]](static_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cchar%20*%3E]](buffer) + 0x0000);
int *B = reinterpret_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cint%20*%3E]](static_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cchar%20*%3E]](buffer) + 0x1000);
int *C = reinterpret_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cint%20*%3E]](static_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cchar%20*%3E]](buffer) + 0x2000);

// 4. 입력 행렬 A, B 초기화 및 C 클리어
for (int i = 0; i < 16 * 16; i++) {
    A[i] = i % 10;
    B[i] = i % 10;
    C[i] = 0;
}

// 5. 64비트 물리 버퍼 주소 전달 헬퍼 함수
auto write_reg64 = [](volatile uint32_t *r, uint32_t offset, uint64_t val) {
    r[offset / 4]     = static_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cuint32_t%3E]](val & 0xFFFFFFFF);
    r[(offset + 4) / 4] = static_cast[[ORCA_RICH_MD:dea7dc2cb5b640c2e3badb7c7e442224:inline-html:%3Cuint32_t%3E]]((val >> 32) & 0xFFFFFFFF);
};

write_reg64(regs, 0x10, 0x70000000ULL); // Matrix A 물리 주소
write_reg64(regs, 0x1C, 0x70001000ULL); // Matrix B 물리 주소
write_reg64(regs, 0x28, 0x70002000ULL); // Matrix C 물리 주소

// 6. 행렬 차원 설정
regs[0x34 / 4] = 16; // a_row
regs[0x3C / 4] = 16; // a_col
regs[0x44 / 4] = 16; // b_col

// 7. 가속기 시작 (AP_START = 1)
regs[0x00 / 4] = 0x01;

// 8. 연산 완료 대기 (AP_DONE 폴링)
while ((regs[0x00 / 4] & 0x02) == 0) {
    // wait for AP_DONE
}

// 9. 결과 확인: C[i]에 FPGA 연산 결과가 즉시 반영됨
```

### 15.3 Python 가속기 제어 핵심 코드 및 동작 분석

```python
import os, mmap, struct

# 1. UIO 파일 열기 및 메모리 매핑
ctrl_fd = os.open("/dev/uio1", os.O_RDWR)
ctrl = mmap.mmap(ctrl_fd, 0x10000, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)

buf_fd = os.open("/dev/uio0", os.O_RDWR)
buf = mmap.mmap(buf_fd, 0x01000000, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)

# 2. 32비트 및 64비트 레지스터 R/W 헬퍼
def write32(mm, offset, val):
    struct.pack_into("<I", mm, offset, val)

def read32(mm, offset):
    return struct.unpack_from("<I", mm, offset)[0]

def write64(mm, offset, val):
    write32(mm, offset, val & 0xFFFFFFFF)
    write32(mm, offset + 4, (val >> 32) & 0xFFFFFFFF)

# 3. 입력 데이터 작성 (DDR)
for i in range(16 * 16):
    struct.pack_into("<i", buf, 0x0000 + i * 4, i % 10)
    struct.pack_into("<i", buf, 0x1000 + i * 4, i % 10)
    struct.pack_into("<i", buf, 0x2000 + i * 4, 0)

# 4. 물리 주소 및 행렬 차원 설정
write64(ctrl, 0x10, 0x70000000)
write64(ctrl, 0x1C, 0x70001000)
write64(ctrl, 0x28, 0x70002000)
write32(ctrl, 0x34, 16)
write32(ctrl, 0x3C, 16)
write32(ctrl, 0x44, 16)

# 5. 가속기 시작 및 AP_DONE 폴링
write32(ctrl, 0x00, 0x01)
while (read32(ctrl, 0x00) & 0x02) == 0:
    pass

# 6. 결과 판독
result = [struct.unpack_from("<i", buf, 0x2000 + i * 4)[0] for i in range(16 * 16)]
```

### 15.4 C++과 Python의 동일 하드웨어 트랜잭션 비교

두 언어 모두 유저스페이스에서 `mmap()`을 통해 동일한 하드웨어 레지스터 및 물리 DDR에 접근하므로, 하드웨어 관점에서 발생하는 AXI 트랜잭션은 100% 동일합니다.

### 15.5 C++ 벤치마크 애플리케이션 실행 결과

```bash
mmult-app
```

출력 결과:

```text
========================================
ZCU104 PetaLinux Matrix Multiplication
Timing Benchmark
========================================
Control device : /dev/uio1
Buffer device  : /dev/uio0
Matrix size    : 16 x 16
CPU runs       : 1000
FPGA runs      : 1000

Benchmarking CPU...
CPU checksum   : 81240
Initial AP_CTRL: 0x00000004
AP_IDLE        : 1
AP_READY       : 0
FPGA warm-up...
Benchmarking FPGA...

========================================
TIMING RESULTS
========================================
CPU software:
  Average : 27.192 us
  Minimum : 27.080 us
  Maximum : 57.880 us

FPGA accelerator:
  Average : 10.042 us
  Minimum :  9.990 us
  Maximum : 34.000 us

Speedup (CPU / FPGA): 2.708x
Result check: PASSED
```

### 15.6 하드웨어 연산 결과 검증 로직

애플리케이션은 ARM Cortex-A53에서 직접 3중 루프로 계산한 소프트웨어 참조 결과($C_{sw}$)와 FPGA 가속기가 DDR에 기록한 결과($C_{hw}$)를 모든 원소에 대해 1:1 비교합니다:

```cpp
bool pass = true;
for (int i = 0; i < 16 * 16; i++) {
    if (C_hw[i] != C_sw[i]) {
        pass = false;
        break;
    }
}
printf("Result check: %s\n", pass ? "PASSED" : "FAILED");
```

- **`Result check: PASSED`**의 출력은 단순한 알고리즘 검증을 넘어 **AXI4-Lite 제어, AXI4 HP DMA 버퍼, 예약 DDR 메모리, UIO 드라이버, mmap 가상 메모리 변환 전체 파이프라인이 완벽하게 정상 동작함**을 보증합니다.

### 15.7 Python 기능 테스트 실행

Python 스크립트를 통해 대화형 디버깅 및 신속한 기능 검증을 수행할 수 있습니다.

### 15.8 성능 측정 체계

1,000회 반복 실행을 통해 평균, 최소, 최대 실행 시간을 통계적으로 측정하여 측정 노이즈를 배제합니다.

### 15.9 CPU 연산 시간 측정 방식

C++ `std::chrono::high_resolution_clock`을 사용하여 순수 ARM AArch64 네이티브 기계어 3중 루프 행렬 곱셈 실행 시간을 측정합니다.

### 15.10 FPGA 가속기 실행 시간 측정 방식

`AP_START` 레지스터 쓰기 직전부터 `AP_DONE` 폴링 완료 직후까지의 전체 하드웨어 구동 시간을 측정합니다. (MMIO 레지스터 접근 오버헤드 포함)

### 15.11 성능 측정 요소 구분

전체 가속기 소요 시간은 다음 요소들로 구성됩니다:

- 레지스터 설정 시간 (MMIO write latency)
- FPGA 연산 및 DDR DMA 읽기/쓰기 시간 (Hardware execution)
- 완료 상태 폴링 지연 시간 (MMIO read latency)

### 15.12 왜 CPU 기준선으로 Python을 사용하면 안 되는가?

Python으로 작성한 CPU 행렬 곱셈은 Python 인터프리터 오버헤드로 인해 네이티브 C++대비 수십~수백 배 느립니다.++  
++따라서 Python CPU 시간과 FPGA 가속 시간을 비교하면 FPGA의 실제 가속 성능이 과도하게 과장(Exaggerate)되므로, **정확한 성능 평가 기준선(Baseline)은 반드시 네이티브 C++ 측정값을 사용해야 합니다.**

### 15.13 폴링(Polling) vs 인터럽트 구동(Interrupt-Driven) 비교

- **폴링 방식 (Polling):** CPU가 `AP_DONE` 비트를 계속해서 읽으며 대기합니다. 연산 시간이 매우 짧은 커널(수십 마이크로초 이하)에서는 컨텍스트 스위칭 오버헤드가 없어 오히려 지연 시간이 가장 짧습니다.
- **인터럽트 구동 방식 (Interrupt-Driven):** 연산 시간이 긴 대형 작업(수 밀리초 이상)에서는 CPU가 `read(uio_fd)`로 블록되어 대기하고, FPGA 완료 인터럽트가 발생하면 커널이 프로세스를 깨워 CPU 점유율을 대폭 절약할 수 있습니다.

---

## 16. 최종 검증 및 요약 (Final Validation &amp; Summary)

### 전체 시스템 하드웨어/소프트웨어 아키텍처 최종 요약

```text
+-----------------------------------------------------------------------------------+
|                        ARM Cortex-A53 (PetaLinux 2024.1)                          |
|                                                                                   |
|  [ Linux Userspace Application: mmult-app ]                                       |
|          |                                   |                                    |
|          v (mmap)                            v (mmap)                             |
|    /dev/uio1 (mmult-control)           /dev/uio0 (mmult-buffer)                    |
|          |                                   |                                    |
|          v                                   v                                    |
|  물리 주소 0xA0000000 (AXI-Lite)       물리 주소 0x70000000 (Reserved DDR)        |
|          |                                   ^                                    |
|          | S_AXI_CONTROL                     | S_AXI_HP0_FPD                      |
|          v                                   | (Direct Memory Access)             |
|  +-------------------------------------------+---------------------------------+  |
|  |             FPGA Matrix Multiplication Accelerator IP (HLS)                 |  |
|  +-----------------------------------------------------------------------------+  |
+-----------------------------------------------------------------------------------+
```

### 13대 핵심 단계 완료 체크리스트


| 단계     | 항목                                                             | 검증 상태    |
| :------: | :-------------------------------------------------------------- | :--------: |
| **1**  | Vitis HLS 16x16 행렬 곱셈 가속기 IP 생성 및 RTL Export                   | **PASS** |
| **2**  | Vivado AXI Interconnect, SmartConnect, Interrupt 통합 및 비트스트림 생성 | **PASS** |
| **3**  | Vivado XSA 하드웨어 플랫폼 파일 PetaLinux 임포트                           | **PASS** |
| **4**  | PetaLinux DTG를 통한 디바이스 트리 자동 생성 및 노드 검출 확인                     | **PASS** |
| **5**  | `system-user.dtsi`를 통한 가속기 제어 레지스터 Generic UIO 오버라이드           | **PASS** |
| **6**  | `system-user.dtsi`를 통한 16 MiB 물리 DDR 영역 예약 (`no-map`)          | **PASS** |
| **7**  | 예약된 DDR 영역의 UIO 노드(`mmult-buffer`) 노출                          | **PASS** |
| **8**  | 리눅스 커널 Generic UIO 드라이버 빌트인 활성화 (`bsp.cfg` 검증)                 | **PASS** |
| **9**  | 커널 부트 아규먼트에 `uio_pdrv_genirq.of_id=generic-uio` 등록             | **PASS** |
| **10** | C++ 유저스페이스 벤치마크 애플리케이션 레시피 생성 및 크로스 컴파일                        | **PASS** |
| **11** | ZCU104 보드 상에서 UIO 디바이스 및 물리 메모리 매핑 검증                          | **PASS** |
| **12** | FPGA 가속기 실행 및 ARM CPU 소프트웨어 연산 결과 1:1 일치 검증 (`PASSED`)         | **PASS** |
| **13** | CPU 대비 FPGA 속도 향상비(Speedup) 측정 및 성능 인프라 검증 완료                  | **PASS** |


---

## 부록 A. PetaLinux에 추가 소프트웨어 패키지 추가 (Adding Extra Software Packages)

기본 PetaLinux 시스템은 임베디드 타깃에 최적화되어 있으므로 가볍고 콤팩트하지만, 필요에 따라 다양한 개발 도구 및 런타임을 포함할 수 있습니다.

### A.1 패키지 설치 여부 사전 확인

보드 콘솔에서 대상 패키지가 이미 설치되어 있는지 확인합니다:

```bash
which python3 && python3 --version
which gcc && gcc --version
which g++ && g++ --version
```

### A.2 루트 파일시스템 설정 메뉴 (petalinux-config -c rootfs)

호스트 개발 환경에서 PetaLinux rootfs 설정 메뉴를 엽니다:

```bash
source ~/petalinux/2024.1/settings.sh
cd ~/project/xilinx-zcu104-2024.1
petalinux-config -c rootfs
```

- 메뉴 검색 단축키: `**/**` 키를 눌러 패키지명을 검색합니다.

### A.3 Python 3 패키지 활성화

`python3`를 검색하여 인터프리터 패키지를 활성화합니다.  
기본 UIO 제어에는 `mmap`, `struct`, `time`, `os`, `glob` 등 표준 라이브러리만으로 충분합니다.

### A.4 GCC / G++ 패키지 활성화 (보드 내 네이티브 컴파일)

보드 상에서 직접 C++소스를 컴파일하고 실험하고자 할 경우 `packagegroup-core-buildessential` 또는 `gcc`, `g++` 패키지를 활성화합니다.  
보드 상에서 컴파일:

```bash
g++ test.cpp -O2 -o test
./test
```

### A.5 기타 유용한 개발 패키지

필요에 따라 다음 패키지들을 검색하여 추가할 수 있습니다:

- `git`, `make`, `cmake`
- `nano`, `vim`
- `openssh`, `rsync`
- `wget`, `curl`

### A.6 선택된 패키지 설정 확인

```bash
grep -i "python\|gcc\|git" project-spec/configs/rootfs_config
```

### A.7 루트 파일시스템 크기 및 메모리 최적화 고려사항

- **개발 환경 (Development Image):** Python, GCC/G++, Git, 디버거 등을 포함하여 편리한 온보드 디버깅 환경 구성
- **양산 배포 환경 (Production Image):** 불필요한 컴파일러 및 툴을 제거하고 최소 실행 런타임만 포함하여 부팅 시간 단축 및 보안/메모리 사용량 최소화

