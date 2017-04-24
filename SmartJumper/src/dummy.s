	.file	"dummy.c"
	.machine power8
	.abiversion 2
	.section	".toc","aw"
	.section	".text"
	.section	".toc","aw"
.LC0:
	.quad	_seed
	.section	".text"
	.align 2
	.p2align 4,,15
	.globl getNextNumber
	.type	getNextNumber, @function
getNextNumber:
0:	addis 2,12,.TOC.-0b@ha
	addi 2,2,.TOC.-0b@l
	.localentry	getNextNumber,.-getNextNumber
	addis 10,2,.LC0@toc@ha		# gpr load fusion, type long
	ld 10,.LC0@toc@l(10)
	ld 9,0(10)
	sldi 8,9,31
	subf 9,9,8
	addis 9,9,0x1
	addi 9,9,-17265
	std 9,0(10)
	ori 2,2,0
	ld 3,0(10)
	blr
	.long 0
	.byte 0,0,0,0,0,0,0,0
	.size	getNextNumber,.-getNextNumber
	.comm	_seed,8,8
	.ident	"GCC: (GNU) 5.2.0"
	.section	.note.GNU-stack,"",@progbits
