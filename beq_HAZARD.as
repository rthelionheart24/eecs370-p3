	lw	0	1	four
	lw	0	3	seven
	beq	0	0	start
	noop
start	add	1	3	4
	nor	3	4	5
	beq	4	5	end
	lw	0	6	4
	noop
	noop
end	halt
four	.fill	4
seven	.fill	7
