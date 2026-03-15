#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>       
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>

// This map acts as the bridge. It holds the file descriptors of your user-space AF_XDP sockets.
struct {
    __uint(type, BPF_MAP_TYPE_XSKMAP);
    __uint(max_entries, 64);
    __type(key, int);
    __type(value, int);
} xsks_map SEC(".maps");

// Target UDP Port (e.g., 1234) we want to bypass the kernel
#define TARGET_PORT 1234

SEC("xdp")
int xdp_pass_udp(struct xdp_md *ctx) {
    // xdp_md gives us direct memory pointers to the raw packet on the NIC
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;

    // 1. Parse Ethernet Header
    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end)
        return XDP_PASS; // Packet too small, let Linux handle it

    // We only care about IPv4 right now
    if (eth->h_proto != __constant_htons(ETH_P_IP))
        return XDP_PASS;

    // 2. Parse IP Header
    struct iphdr *iph = (struct iphdr *)(eth + 1);
    if ((void *)(iph + 1) > data_end)
        return XDP_PASS;

    // We only care about UDP
    if (iph->protocol != IPPROTO_UDP)
        return XDP_PASS;

    // 3. Parse UDP Header
    struct udphdr *udph = (struct udphdr *)(iph + 1);
    if ((void *)(udph + 1) > data_end)
        return XDP_PASS;

    // 4. The Routing Logic
    // If the packet hits our target port, redirect it to our AF_XDP socket map
    if (udph->dest == __constant_htons(TARGET_PORT)) {
        // Redirect to the socket attached to the same RX queue index
        return bpf_redirect_map(&xsks_map, ctx->rx_queue_index, XDP_PASS);
    }

    // Otherwise, let the normal Linux TCP/IP stack handle it
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
