	lw	0	1	five
	lw	0	3	seven
	beq	0	0	start
	noop
	noop
	noop
	noop
	noop
start	add	1	2	5
	beq	0	4	end
	nor	0	3	6
end	halt
five	.fill	5
seven	.fill	7
