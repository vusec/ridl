#include <stdio.h>

#include <vuln/l1tf.h>
#include <vuln/meltdown.h>
#include <vuln/ridl.h>
#include <vuln/spectre.h>

#include "../system.h"

void
show_system_info(struct sys_info *info);
void
show_meltdown_info(struct meltdown_info *info);
void
show_l1tf_info(struct l1tf_info *info);
void
show_spectre_v1_info(struct spectre_info *info);
void
show_spectre_v2_info(struct spectre_info *info);
void
show_ridl_info(struct ridl_info *info);

int
main(void)
{
	struct sys_info sys_info;
	struct spectre_info spectre_info;
	struct meltdown_info meltdown_info;
	struct l1tf_info l1tf_info;
	struct ridl_info ridl_info;

	query_sys_info(&sys_info);
	query_spectre_info(&spectre_info);
	query_meltdown_info(&meltdown_info);
	query_l1tf_info(&l1tf_info);
	query_ridl_info(&ridl_info);

	show_system_info(&sys_info);
	printf("\n");
	show_spectre_v1_info(&spectre_info);
	printf("\n");
	show_spectre_v2_info(&spectre_info);
	printf("\n");
	show_meltdown_info(&meltdown_info);
	printf("\n");
	show_l1tf_info(&l1tf_info);
	printf("\n");
	show_ridl_info(&ridl_info);

	return 0;
}
