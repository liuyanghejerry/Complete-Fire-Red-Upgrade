.text
.align 2
.thumb

.include "../asm_defines.s"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Game Speed Up
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x890, 0xFF
main:
	ldr r2, .SuperBits
	ldrh r3, [r2]
	mov r0, #1	
	bic r3, r0
	strh r3, [r2]
loop_label:
	swi #2
	ldrh r3, [r2]
	tst r3, r0
	beq loop_label
	bx lr

.align 2
.SuperBits: .word 0x0300310C

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Dynamic Overworld Palette - part of hook at 0x779c
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x779A, 0xff
	.byte 0x0, 0x0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Pokedex Flags Banned Battle Types
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.org 0x13248, 0xFF
.word BATTLE_WIRELESS | BATTLE_OLD_MAN | BATTLE_E_READER | BATTLE_GHOST | BATTLE_TRAINER_TOWER | BATTLE_FRONTIER

.org 0x13324, 0xFF
.word BATTLE_WIRELESS | BATTLE_OLD_MAN | BATTLE_E_READER | BATTLE_GHOST | BATTLE_TRAINER_TOWER | BATTLE_FRONTIER

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Safari Zone Ball Count
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x16ADA, 0xFF
	ldrh r0, [r1]
	sub r0, #0x1
	strh r0, [r1]

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Limiters
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x21CFA, 0xFF
MaxLevelChange2:
	.byte MAX_LEVEL

.org 0x21FB6, 0xFF
MaxLevelChange3:
	.byte MAX_LEVEL

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Pokedex Flags Banned Battle Types
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.org 0x24178, 0xFF
.word BATTLE_WIRELESS | BATTLE_OLD_MAN | BATTLE_E_READER | BATTLE_GHOST | BATTLE_TRAINER_TOWER | BATTLE_FRONTIER

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Hidden Abilities - Various Something
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x026E34, 0xFF
HiddenAbilityChange2:
	mov r8, r8
	mov r0, r4 @mon
	bl HiddenAbilityChange2 + 0x19F04 @0x08040D38
	lsl r0, r0, #0x18
	lsr r0, r0, #0x18

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Hidden Abilities - Various Something
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x026ECC, 0xFF
HiddenAbilityChange3:
	and r0, r1
	cmp r0, #0x0
	beq HiddenAbilityChange3 + 0x20
	mov r8, r8
	mov r0, r4 @mon
	bl HiddenAbilityChange3 + 0x19E6C @0x08040D38
	lsl r0, r0, #0x18
	lsr r0, r0, #0x18
	cmp r0, #ABILITY_SOUNDPROOF
	beq HiddenAbilityChange3 + 0x20
	mov r0, #0x1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Hidden Abilities - Player
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x02A800, 0xFF
HiddenAbilityChange4_1:
	mov r1, #0x41 @species2
	bl HiddenAbilityChange4_1 + 0x153E8 @get_attr
	lsl r0, r0, #0x10
	lsr r5, r0, #0x10
	mov r8, r8
	mov r8, r8
	mov r8, r8
	mov r8, r8
	mov r8, r8
	mov r8, r8

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Hidden Abilities - Opponent
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x02A890, 0xFF
HiddenAbilityChange4_2:
	mov r8, r8
	mov r0, r4
	bl HiddenAbilityChange4_2 + 0x164A8 @0x08040D38
	lsl r0, #0x18
	lsr r0, #0x18

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Limiter
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x32F6E, 0xFF
MaxLevelChange14:
	.byte MAX_LEVEL - 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Limiters
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x3E806, 0xFF
MaxLevelChange4:
	.byte MAX_LEVEL

.org 0x3E872, 0xFF
MaxLevelChange5:
	.byte MAX_LEVEL

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Hidden Abilities - Summary Screen
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x041318, 0xFF
@poke summary info
HiddenAbilityChange5:
	mov r8, r8
	mov r8, r8
	mov r0, r4 @mon
	bl HiddenAbilityChange5 - 0x5E0 @0x08040D38
	mov r1, r7
	add r1, #0x20
	strb r0, [r1]

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Limiters
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x41834, 0xFF
MaxLevelChange6:
	.byte MAX_LEVEL

.org 0x41B0E, 0xFF
MaxLevelChange13:
	.byte MAX_LEVEL - 1

.org 0x41B1E, 0xFF
MaxLevelChange7:
	.byte MAX_LEVEL, 0x29, 0x6, 0xD9, MAX_LEVEL

@These commented out values refer to stat boosting items,
@NOT max level!
@.org 0x420E8, 0xFF
@MaxLevelChange9:
@	.byte MAX_LEVEL

@.org 0x420EC, 0xFF
@MaxLevelChange10:
@	.byte MAX_LEVEL

.org 0x4274E, 0xFF
MaxLevelChange11:
	.byte MAX_LEVEL

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level - Limiter
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x45684, 0xFF
MaxLevelChange12:
	.byte MAX_LEVEL

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Hack - Limiter
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x4A216, 0xFF
MaxLevelChange1:
	.byte MAX_LEVEL	
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Character Customization
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.org 0x588C6, 0xFF	@16 bit sprite ids
	lsl r0, #0x10
	lsr r0, #0x10

.org 0x5C7D6, 0xFF	@16 bit sprite ids
	lsl r0, #0x10
	lsr r0, #0x10 

.org 0x5CB88, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x5CBCC, 0xFF	@16 bit sprite ids
	lsl r0, #0x10
	lsr r0, #0x10

.org 0x5CBFE, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x5D268, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x5D716, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x5D916, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x5e5d4, 0xff
	.byte 0x8, 0x47		@ routine_ptr+1 at 5e5f4
	
.org 0x5e744, 0xff		@ fix size
	lsl r4, r4, #0x10
	lsr r4, r4, #0x10
	lsl r5, r5, #0x10
	lsr r5, r5, #0x10
	mov r0, #0x0
	str r0, [sp, #0x20]
	mov r1, r9
	ldrb r0, [r1, #0x1]
	ldrb r1, [r1, #0x3]
	lsl r1, r1, #0x18
	lsr r1, r1, #0x10
	orr r0, r1	

.org 0x5e8f0, 0xff		@ change size
	lsl r0, r0, #0x10
	lsr r0, r0, #0x10
	
.org 0x5e962, 0xff
	mov r8, r8

.org 0x5ea0a, 0xff		@ fix createsprite
	lsl r5, #0x10
	lsr r5, #0x10	

.org 0x5ee84, 0xff		@ fix link npcs
	.byte 0x8, 0x47

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Dynamic Overworld Palettes & More OW Sprites
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.org 0x5F06E, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x5f093, 0xff	@don't auto-load NPC palettes into slot 0 or 0xA
	.byte 0xE0

.org 0x5f114, 0xff	@don't reset pal slot during player animation or reflection
	.hword 0x0

.org 0x5f4ba, 0xff	@remove byte casts for NPC palette number
	.byte 0x1, 0x0, 0x0, 0x0

.org 0x5f548, 0xff	@remove byte casts for NPC palette number
	.byte 0xc0, 0x0, 0x9, 0x49
	
.org 0x5f54c, 0xff	@remove byte casts for NPC palette number
	.byte 0x0, 0x0
	
.org 0x5f5d2, 0xff	@remove byte casts for NPC palette number
	.byte 0x2, 0x0, 0x0, 0x0

.org 0x5f5e8, 0xff	@don't auto-load NPC palettes into slot 0 or 0xA
	.byte 0x70, 0x47
		
.org 0x5f658, 0xff	@don't auto-load NPC palettes into slot 0 or 0xA
	.byte 0x70, 0x47
	
.org 0x79c18, 0xff	@don't load rain palette on entering map
	.byte 0x0, 0x25, 0xe, 0xe0

.org 0x7a31f, 0xff	@don't treat slot 0xC as weather palette
	.byte 0xe0
	
.org 0x7a85e, 0xff	@make it compatible with daniilS' snowflakes routine
	.byte 0x80, 0x18
	
.org 0x7a872, 0xff	@make it compatible with daniilS' snowflakes routine
	.byte 0x1, 0x22, 0x5, 0xe0
		
.org 0x7aae7, 0xff	@don't record brightened slots
	.byte 0xe0

.org 0x86ADA, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x86CB4, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x86CC8, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x87024, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x87522, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x877BC, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ GetBagItemQuanity
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x99DA0, 0xFF
	ldrh r0, [r0]
	bx lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Triple Layer Blocks
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x9B868, 0xFF
	.byte 0x2
	
.org 0x9B86E, 0xFF
	.byte 0x2f, 0xd0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ More OW Sprites
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x9EF68, 0xFF
	lsl r0, #0x10
	lsr r0, #0x10 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Safari Zone Ball Count
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0xA0EF6, 0xFF
	strh r0, [r1]
	
.org 0xA0F50, 0xFF
	ldrh r0, [r0]

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Dynamic Overworld Palettes & More OW Sprites
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0xC97AC, 0xFF	@16 bit sprite ids
	lsl r5, #0x10
	lsr r5, #0x10

.org 0xC9950, 0xFF	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0xdaec4, 0xff	@@don't auto-load NPC palettes into slot 0 or 0xA
	.byte 0x70, 0x47
	
.org 0xdaf20, 0xff	@don't auto-load NPC palettes into slot 0 or 0xA
	.byte 0x70, 0x47
	
.org 0xdafb8, 0xff	@don't reset pal slot during player animation or reflection
	.hword 0x0
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Remove TM Animation
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x11CA2C, 0xFF
	.byte 0x00, 0xF0, 0x0E, 0xFA
	
.org 0x11CD9E, 0xFF
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0xE0

.org 0x11CE6E, 0xFF
	.byte 0x0, 0x0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Hack - Rare Candies
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x1262D2, 0xFF
MaxLevelRareCandies:
	.byte MAX_LEVEL

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Hack - Summary Screen
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x136684, 0xFF
SummaryScreenExpDisplay1:
	.byte MAX_LEVEL - 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Hidden Abilities - Ability Names
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x1366EC, 0xFF
HiddenAbilityChange5_SummaryScreen:
	ldr r0, [r6]
	add r0, r8
	bl HiddenAbilityChange5_SummaryScreen - 0xF59B4 @0x08040D38
	lsl r0, r0, #0x18
	lsr r4, r0, #0x18
	ldr r0, [r6]
	b HiddenAbilityChange5_SummaryScreen + 0x24 @0x08136710
.hword 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Max Level Hack - Summary Screen Exp Display
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x13A9DC, 0xFF
SummaryScreenExpDisplay2:
	.byte MAX_LEVEL - 1, 0x2D, 0x0, 0xD9

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Dynamic Overworld Palettes & More OW Sprites
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x1504C0, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x150500, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x15059C, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x15063C, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x1506C4, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x1506D8, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x15072A, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x1507BE, 0xff	@16 bit sprite ids
	lsl r1, #0x10
	lsr r1, #0x10

.org 0x1d9895, 0xff		@don't load sand palette on healing
	.byte 0x7, 0x9c, 0xbe, 0x3c, 0x8, 0x8d, 0x3b, 0x8, 0x8, 0x4
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Triple Layer Blocks
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x352F0B, 0xFF
	.byte 0x70
	
.org 0x352F16, 0xFF
	.byte 0x1C
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Multichoice Pointers
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x3E05B0, 0xFF
.word MULTICHOICE_STRING_LOADER
.word 0x2
.word MULTICHOICE_STRING_LOADER
.word 0x3
.word MULTICHOICE_STRING_LOADER
.word 0x4
.word MULTICHOICE_STRING_LOADER
.word 0x5
.word MULTICHOICE_STRING_LOADER
.word 0x6
.word MULTICHOICE_STRING_LOADER
.word 0x7

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Dynamic Overworld Palettes
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.org 0x45fd52, 0xff		@pal slot of scroll arrow in Fame Checker
	.byte 0x9
