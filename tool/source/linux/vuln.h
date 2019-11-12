#pragma once

struct vuln_iter {
	char *line, *next, *key, *value;
};

int
iter_vuln(struct vuln_iter *iter, const char *name);
int
next_vuln(struct vuln_iter *iter);
