	lw	0	7	two
	lw	0	1	one
	beq	0	0	start
	noop
	add	0	3	4
start	beq	2	7	end
	add	2	1	2
	beq	0	0	start
end	halt
one	.fill	1
two	.fill	2
