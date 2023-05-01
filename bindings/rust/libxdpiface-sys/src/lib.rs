#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;
    use std::alloc::{alloc, dealloc, Layout};
    use std::ptr;
    use std::slice;
    use std::convert::TryInto;
    use hexdump::hexdump;

    #[test]
    fn xdp_iface_constructor_destructor() {
        /* Those constatns are still not ported */
        const SO_PREFER_BUSY_POLL: i32 = 69;
        const SO_BUSY_POLL: i32 = 36;
        const SO_BUSY_POLL_BUDGET: i32 = 70;

        unsafe {
            let interface = CString::new("lo").unwrap();
            let xdp_prog = CString::new("./../../../build/xdp_sock_bpf.o").unwrap();
            let xdp_xsks_map = CString::new("xsks_map").unwrap();
            const batch_size: u32 = 30;

            let mut xdp_iface: *mut xdp_iface_t = xdp_iface_new(interface.as_ptr() as *const i8);
            xdp_iface_load_program(xdp_iface, xdp_prog.as_ptr() as *const i8);

            let mut xdp_sock: *mut xdp_sock_t = xdp_sock_new(xdp_iface);
            xdp_sock_lookup_bpf_map(xdp_sock, xdp_iface, xdp_xsks_map.as_ptr() as *const i8 , 4, 4);

            xdp_sock_set_sockopt(xdp_sock, SO_PREFER_BUSY_POLL, 1);
            xdp_sock_set_sockopt(xdp_sock, SO_BUSY_POLL, 20);
            xdp_sock_set_sockopt(xdp_sock, SO_BUSY_POLL_BUDGET, batch_size as i32);

            let o_buffer_size = 1500;
            let pattern = 0x55;
            let layout = Layout::from_size_align(o_buffer_size, 1).unwrap();
            let o_buffer = alloc(layout) as *mut i8;
            ptr::write_bytes(o_buffer, pattern, o_buffer_size);

            xdp_sock_tx_batch_set_size(xdp_sock, batch_size);
            for _ in 0..batch_size {
                xdp_sock_send(xdp_sock, o_buffer, o_buffer_size as u64);
            }
            xdp_sock_tx_batch_release(xdp_sock, batch_size);

            let mut frames_rcvd: u32 = 0;
            let mut i_buffer_size: u64 = 0;
            let i_buffer = alloc(layout) as *mut i8;

            xdp_sock_rx_batch_get_size(xdp_sock, &mut frames_rcvd, batch_size);
            for _ in 0..frames_rcvd {
                xdp_sock_recv(xdp_sock, i_buffer, &mut i_buffer_size);
                hexdump(slice::from_raw_parts(i_buffer as *const u8, i_buffer_size as usize));
            }
            xdp_sock_rx_batch_release(xdp_sock, frames_rcvd);

            dealloc(o_buffer as *mut u8, layout);
            dealloc(i_buffer as *mut u8, layout);

            xdp_iface_unload_program(xdp_iface);

            xdp_sock_destroy(&mut xdp_sock as *mut *mut xdp_sock_t);
            xdp_iface_destroy(&mut xdp_iface  as *mut *mut xdp_iface_t);
        }
    }
}