	lw	0	1	five
	beq	0	0	start
start	beq	0	0	mid
	nor	0	0	4
mid	beq	0	0	2
	add	0	0	5
	add	2	5	2
	lw	0	3	2
	halt
five	.fill	5
