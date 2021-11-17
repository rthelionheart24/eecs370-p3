	sw	0	0	0
	lw	0	1	five
	sw	0	1	one
	lw	0	2	two
	noop
	sw	0	2	ten
	add	0	1	2
	nor	5	6	1
	add	0	3	4
	sw	1	2	four
	halt
one	.fill	1
two	.fill	2
three	.fill	3
four	.fill	4
five	.fill	5
six	.fill	6
seven	.fill	7
eight	.fill	8
ten	.fill	10
