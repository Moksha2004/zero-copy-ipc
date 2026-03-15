#include <iostream>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <net/if.h>
#include <unistd.h>
#include <csignal>

// Global pointer for cleanup
struct bpf_link *global_link = nullptr;
struct bpf_object *global_obj = nullptr;

void handle_signal(int sig) {
    std::cout << "\nDetaching XDP program and cleaning up..." << std::endl;
    if (global_link) bpf_link__destroy(global_link);
    if (global_obj) bpf_object__close(global_obj);
    exit(0);
}

int main() {
    // 1. Identify the network interface (eth0 from your ifconfig output)
    int ifindex = if_nametoindex("eth0");
    if (ifindex == 0) {
        std::cerr << "[ERROR] Failed to find eth0 interface. Check ifconfig." << std::endl;
        return 1;
    }

    // 2. Open the eBPF object file we just compiled
    struct bpf_object *obj = bpf_object__open_file("xdp_prog.o", NULL);
    if (libbpf_get_error(obj)) {
        std::cerr << "[ERROR] Failed to open xdp_prog.o" << std::endl;
        return 1;
    }
    global_obj = obj;

    // 3. Load the object into the kernel
    if (bpf_object__load(obj)) {
        std::cerr << "[ERROR] Failed to load BPF object into kernel." << std::endl;
        return 1;
    }

    // 4. Find our specific function inside the C code
    struct bpf_program *prog = bpf_object__find_program_by_name(obj, "xdp_pass_udp");
    if (!prog) {
        std::cerr << "[ERROR] Failed to find program 'xdp_pass_udp'" << std::endl;
        return 1;
    }

    // 5. Attach the program to eth0
    // Note: Because you are on WSL2/Hyper-V, this will silently fall back to SKB mode
    struct bpf_link *link = bpf_program__attach_xdp(prog, ifindex);
    if (libbpf_get_error(link)) {
        std::cerr << "[ERROR] Failed to attach XDP program to eth0. Are you running as root?" << std::endl;
        return 1;
    }
    global_link = link;

    // Handle Ctrl+C gracefully so we don't leave zombie hooks in the kernel
    std::signal(SIGINT, handle_signal);

    std::cout << "[SUCCESS] XDP program actively attached to eth0!" << std::endl;
    std::cout << "[INFO] Waiting for packets. Press Ctrl+C to detach and exit." << std::endl;

    // Keep the process alive
    while (true) {
        sleep(1);
    }

    return 0;
}
