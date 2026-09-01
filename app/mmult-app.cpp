#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dirent.h>
#include <time.h>

#include <fstream>
#include <string>
#include <limits>

// ============================================================
// Configuration
// ============================================================

static constexpr int MATRIX_SIZE = 16;
static constexpr int NUM_ELEMENTS = MATRIX_SIZE * MATRIX_SIZE;

static constexpr int CPU_RUNS  = 1000;
static constexpr int FPGA_RUNS = 1000;

// ============================================================
// Reserved DDR layout
// ============================================================

static constexpr uint64_t A_PHYS = 0x70000000ULL;
static constexpr uint64_t B_PHYS = 0x70001000ULL;
static constexpr uint64_t C_PHYS = 0x70002000ULL;

static constexpr size_t A_OFFSET = 0x0000;
static constexpr size_t B_OFFSET = 0x1000;
static constexpr size_t C_OFFSET = 0x2000;

static constexpr size_t BUFFER_MAP_SIZE = 0x01000000;

// ============================================================
// HLS AXI-Lite register map
// ============================================================

static constexpr uint32_t REG_AP_CTRL = 0x00;

static constexpr uint32_t REG_A       = 0x10;
static constexpr uint32_t REG_B       = 0x1C;
static constexpr uint32_t REG_C       = 0x28;

static constexpr uint32_t REG_A_ROW   = 0x34;
static constexpr uint32_t REG_A_COL   = 0x3C;
static constexpr uint32_t REG_B_COL   = 0x44;

static constexpr uint32_t AP_START = (1U << 0);
static constexpr uint32_t AP_DONE  = (1U << 1);
static constexpr uint32_t AP_IDLE  = (1U << 2);
static constexpr uint32_t AP_READY = (1U << 3);

// ============================================================
// Timing
// ============================================================

static uint64_t get_time_ns()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL
         + static_cast<uint64_t>(ts.tv_nsec);
}

// ============================================================
// Find UIO by linux,uio-name
// ============================================================

static std::string find_uio_device(const std::string& wanted_name)
{
    DIR* dir = opendir("/sys/class/uio");

    if (!dir) {
        perror("opendir");
        return "";
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        std::string dev(entry->d_name);

        if (dev.rfind("uio", 0) != 0)
            continue;

        std::ifstream file(
            "/sys/class/uio/" + dev + "/name");

        if (!file)
            continue;

        std::string name;
        std::getline(file, name);

        if (name == wanted_name) {
            closedir(dir);
            return "/dev/" + dev;
        }
    }

    closedir(dir);
    return "";
}

// ============================================================
// Register access
// ============================================================

static inline void write_reg(
    volatile uint32_t* regs,
    uint32_t offset,
    uint32_t value)
{
    regs[offset / 4] = value;
}

static inline uint32_t read_reg(
    volatile uint32_t* regs,
    uint32_t offset)
{
    return regs[offset / 4];
}

static inline void write_reg64(
    volatile uint32_t* regs,
    uint32_t offset,
    uint64_t value)
{
    write_reg(
        regs,
        offset,
        static_cast<uint32_t>(value));

    write_reg(
        regs,
        offset + 4,
        static_cast<uint32_t>(value >> 32));
}

// ============================================================
// CPU reference
// ============================================================

static void software_mmult(
    const int* A,
    const int* B,
    int* C)
{
    for (int i = 0; i < MATRIX_SIZE; i++) {

        for (int j = 0; j < MATRIX_SIZE; j++) {

            int sum = 0;

            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum +=
                    A[i * MATRIX_SIZE + k] *
                    B[k * MATRIX_SIZE + j];
            }

            C[i * MATRIX_SIZE + j] = sum;
        }
    }
}

// ============================================================
// Run FPGA once
// ============================================================

static bool run_fpga(
    volatile uint32_t* regs)
{
    write_reg(regs, REG_AP_CTRL, AP_START);

    constexpr unsigned long MAX_POLLS = 100000000UL;

    for (unsigned long poll = 0;
         poll < MAX_POLLS;
         poll++) {

        uint32_t status =
            read_reg(regs, REG_AP_CTRL);

        if (status & AP_DONE)
            return true;
    }

    return false;
}

// ============================================================
// Main
// ============================================================

int main()
{
    printf("========================================\n");
    printf(" ZCU104 PetaLinux Matrix Multiplication\n");
    printf(" Timing Benchmark\n");
    printf("========================================\n\n");

    // --------------------------------------------------------
    // Locate UIO devices
    // --------------------------------------------------------

    std::string control_dev =
        find_uio_device("mmult-control");

    std::string buffer_dev =
        find_uio_device("mmult-buffer");

    if (control_dev.empty() || buffer_dev.empty()) {
        fprintf(stderr, "ERROR: UIO devices not found\n");
        return EXIT_FAILURE;
    }

    printf("Control device : %s\n",
           control_dev.c_str());

    printf("Buffer device  : %s\n\n",
           buffer_dev.c_str());

    // --------------------------------------------------------
    // Open/map control
    // --------------------------------------------------------

    int control_fd =
        open(control_dev.c_str(), O_RDWR);

    if (control_fd < 0) {
        perror("open control");
        return EXIT_FAILURE;
    }

    void* control_map =
        mmap(nullptr,
             0x10000,
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             control_fd,
             0);

    if (control_map == MAP_FAILED) {
        perror("mmap control");
        return EXIT_FAILURE;
    }

    volatile uint32_t* regs =
        reinterpret_cast<volatile uint32_t*>(
            control_map);

    // --------------------------------------------------------
    // Open/map reserved DDR
    // --------------------------------------------------------

    int buffer_fd =
        open(buffer_dev.c_str(), O_RDWR);

    if (buffer_fd < 0) {
        perror("open buffer");
        return EXIT_FAILURE;
    }

    void* buffer_map =
        mmap(nullptr,
             BUFFER_MAP_SIZE,
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             buffer_fd,
             0);

    if (buffer_map == MAP_FAILED) {
        perror("mmap buffer");
        return EXIT_FAILURE;
    }

    uint8_t* buffer =
        reinterpret_cast<uint8_t*>(buffer_map);

    volatile int* A_hw =
        reinterpret_cast<volatile int*>(
            buffer + A_OFFSET);

    volatile int* B_hw =
        reinterpret_cast<volatile int*>(
            buffer + B_OFFSET);

    volatile int* C_hw =
        reinterpret_cast<volatile int*>(
            buffer + C_OFFSET);

    // --------------------------------------------------------
    // Software matrices
    // --------------------------------------------------------

    int A_sw[NUM_ELEMENTS];
    int B_sw[NUM_ELEMENTS];
    int C_sw[NUM_ELEMENTS];

    for (int i = 0; i < NUM_ELEMENTS; i++) {

        A_sw[i] = i % 10;
        B_sw[i] = i % 10;
        C_sw[i] = 0;

        A_hw[i] = A_sw[i];
        B_hw[i] = B_sw[i];
        C_hw[i] = 0;
    }

    printf("Matrix size    : %d x %d\n",
           MATRIX_SIZE, MATRIX_SIZE);

    printf("CPU runs       : %d\n", CPU_RUNS);
    printf("FPGA runs      : %d\n\n", FPGA_RUNS);

    // ========================================================
    // CPU benchmark
    // ========================================================

    printf("Benchmarking CPU...\n");

    uint64_t cpu_total_ns = 0;
    uint64_t cpu_min_ns =
        std::numeric_limits<uint64_t>::max();
    uint64_t cpu_max_ns = 0;

    for (int run = 0; run < CPU_RUNS; run++) {

        uint64_t start = get_time_ns();

        software_mmult(A_sw, B_sw, C_sw);

        uint64_t end = get_time_ns();

        uint64_t elapsed = end - start;

        cpu_total_ns += elapsed;

        if (elapsed < cpu_min_ns)
            cpu_min_ns = elapsed;

        if (elapsed > cpu_max_ns)
            cpu_max_ns = elapsed;
    }

    double cpu_avg_ns =
        static_cast<double>(cpu_total_ns)
        / CPU_RUNS;

    // Prevent optimizer from treating result as unused.
    volatile int cpu_checksum = 0;

    for (int i = 0; i < NUM_ELEMENTS; i++)
        cpu_checksum += C_sw[i];

    printf("CPU checksum   : %d\n\n",
           static_cast<int>(cpu_checksum));

    // ========================================================
    // Configure FPGA
    // ========================================================

    write_reg64(regs, REG_A, A_PHYS);
    write_reg64(regs, REG_B, B_PHYS);
    write_reg64(regs, REG_C, C_PHYS);

    write_reg(regs, REG_A_ROW, MATRIX_SIZE);
    write_reg(regs, REG_A_COL, MATRIX_SIZE);
    write_reg(regs, REG_B_COL, MATRIX_SIZE);

    uint32_t ctrl =
        read_reg(regs, REG_AP_CTRL);

    printf("Initial AP_CTRL: 0x%08x\n", ctrl);
    printf("AP_IDLE        : %u\n",
           !!(ctrl & AP_IDLE));
    printf("AP_READY       : %u\n\n",
           !!(ctrl & AP_READY));

    // ========================================================
    // FPGA warm-up
    // ========================================================

    printf("FPGA warm-up...\n");

    if (!run_fpga(regs)) {
        fprintf(stderr,
                "ERROR: FPGA warm-up timeout\n");
        return EXIT_FAILURE;
    }

    // ========================================================
    // FPGA benchmark
    // ========================================================

    printf("Benchmarking FPGA...\n");

    uint64_t fpga_total_ns = 0;
    uint64_t fpga_min_ns =
        std::numeric_limits<uint64_t>::max();
    uint64_t fpga_max_ns = 0;

    for (int run = 0; run < FPGA_RUNS; run++) {

        uint64_t start = get_time_ns();

        if (!run_fpga(regs)) {
            fprintf(stderr,
                    "ERROR: FPGA timeout at run %d\n",
                    run);
            return EXIT_FAILURE;
        }

        uint64_t end = get_time_ns();

        uint64_t elapsed = end - start;

        fpga_total_ns += elapsed;

        if (elapsed < fpga_min_ns)
            fpga_min_ns = elapsed;

        if (elapsed > fpga_max_ns)
            fpga_max_ns = elapsed;
    }

    double fpga_avg_ns =
        static_cast<double>(fpga_total_ns)
        / FPGA_RUNS;

    // ========================================================
    // Verify result
    // ========================================================

    bool match = true;

    for (int i = 0; i < NUM_ELEMENTS; i++) {

        if (C_hw[i] != C_sw[i]) {

            fprintf(stderr,
                    "Mismatch at %d: CPU=%d FPGA=%d\n",
                    i,
                    C_sw[i],
                    C_hw[i]);

            match = false;
            break;
        }
    }

    // ========================================================
    // Results
    // ========================================================

    printf("\n");
    printf("========================================\n");
    printf(" TIMING RESULTS\n");
    printf("========================================\n");

    printf("\nCPU software:\n");
    printf("  Average : %.3f us\n",
           cpu_avg_ns / 1000.0);
    printf("  Minimum : %.3f us\n",
           cpu_min_ns / 1000.0);
    printf("  Maximum : %.3f us\n",
           cpu_max_ns / 1000.0);

    printf("\nFPGA accelerator:\n");
    printf("  Average : %.3f us\n",
           fpga_avg_ns / 1000.0);
    printf("  Minimum : %.3f us\n",
           fpga_min_ns / 1000.0);
    printf("  Maximum : %.3f us\n",
           fpga_max_ns / 1000.0);

    printf("\nSpeedup (CPU / FPGA): %.3fx\n",
           cpu_avg_ns / fpga_avg_ns);

    printf("\nResult check: %s\n",
           match ? "PASSED" : "FAILED");

    printf("========================================\n");

    // --------------------------------------------------------
    // Cleanup
    // --------------------------------------------------------

    munmap(buffer_map, BUFFER_MAP_SIZE);
    munmap(control_map, 0x10000);

    close(buffer_fd);
    close(control_fd);

    return match ? EXIT_SUCCESS : EXIT_FAILURE;
}