/**
 * PHY operating interface, not used.
static void zynqmp_gem_phy_operate(uint8_t phy_addr, uint8_t regnum, uint8_t op, uint16_t *data) {
    while (!zynqmp_gem_netsta_md_rdf(&device)) {
#ifdef ZYNQMP_GEM_DEBUG
        ZYNQMP_GEM_DEBUG("waiting for man done before operation\n");
#endif
    }

    zynqmp_gem_phymng_t content = zynqmp_gem_phymng_default;
    content = zynqmp_gem_phymng_w0_insert(content, 0);
    content = zynqmp_gem_phymng_w1_insert(content, 1);
    content = zynqmp_gem_phymng_op_insert(content, op);
    content = zynqmp_gem_phymng_pa_insert(content, phy_addr);
    content = zynqmp_gem_phymng_ra_insert(content, regnum);
    content = zynqmp_gem_phymng_w10_insert(content, 2);
    content = zynqmp_gem_phymng_pwrd_insert(content, *data);

	// Write phymng and wait for completion
	zynqmp_gem_phymng_wr(&device, content);
    while (!zynqmp_gem_netsta_md_rdf(&device)) {
#ifdef ZYNQMP_GEM_DEBUG
        ZYNQMP_GEM_DEBUG("waiting for man done after operation\n");
#endif
    }

	if (op == zynqmp_gem_c45pri_c22rd)
        *data = zynqmp_gem_phymng_pwrd_rdf(&device);
}
 */

