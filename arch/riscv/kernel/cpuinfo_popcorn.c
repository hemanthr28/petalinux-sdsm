/**
 * @file cpuinfo_popcorn.c
 *
 * Popcorn Linux RISCV64 cpuinfo implementation
 *
 * @author Cesar Philippidis, RASEC Technologies 2020
 */

#include <linux/kernel.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/elf.h>
#include <linux/personality.h>

#include <linux/of.h>
#include <asm/smp.h>

#include <popcorn/cpuinfo.h>
#include <popcorn/pcn_kmsg.h>

static void copy_isa(struct percore_info_riscv *core,
		     struct device_node *node)
{
  static const char *ext = "mafdcsu", *e;
	const char *isa;
	int i = 5;

	strncat(core->isa, "rv64i", 5);

	if (!of_property_read_string(node, "riscv,isa", &isa)) {
		isa += 5; // skip rv64i prefix

		for (e = ext; *e != '\0'; ++e) {
			if (isa[0] == e[0]) {
				if (isa[0] != 's')
					core->isa[i++] = isa[0];
				isa++;
			}
		}
	}
}

static void copy_mmu(struct percore_info_riscv *core,
		     struct device_node *node)
{
	const char *mmu_type;

	if (!of_property_read_string(node, "mmu-type", &mmu_type))
		strcat(core->mmu, mmu_type+6);
	else
		strcat(core->mmu, "unknown");
}

int fill_cpu_info(struct remote_cpu_info *res)
{
	int i;
	unsigned int count = 0;
	struct cpuinfo_arch_riscv *arch = &res->riscv;

	for_each_online_cpu(i) {
		struct percore_info_riscv *core = &arch->cores[count];
		struct device_node *node = of_get_cpu_node(i, NULL);

		core->cpu_id = i;
		core->hart = cpuid_to_hartid_map(i);
		core->isa[0] = '\0';
		core->mmu[0] = '\0';

		copy_isa(core, node);
		copy_mmu(core, node);
	}

	return 0;
}
