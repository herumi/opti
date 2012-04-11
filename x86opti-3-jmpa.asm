segment .text
global _a_old
global a_old
_a_old:
a_old:
	mov eax, [esp + 4]
	cmp eax, 2
	jb old_k_lt_2
	je old_k_eq_2
	mov eax, 100
	ret
old_k_lt_2:
	mov eax, 3
	ret
old_k_eq_2:
	mov eax, 6
	ret

global _a_new
global a_new
_a_new:
a_new:
	mov eax, [esp + 4]
	cmp eax, 2
	je new_k_eq_2
	cmp eax, 3
	je new_k_eq_3
	mov eax, 3
	ret
new_k_eq_3:
	mov eax, 100
	ret
new_k_eq_2:
	mov eax, 6
	ret
