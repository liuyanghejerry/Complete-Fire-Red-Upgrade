#include "defines.h"
#include "defines_battle.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/window.h"
#include "../include/constants/songs.h"

#include "../include/new/accuracy_calc.h"
#include "../include/new/ai_util.h"
#include "../include/new/battle_util.h"
#include "../include/new/damage_calc.h"
#include "../include/new/util.h"
#include "../include/new/general_bs_commands.h"
#include "../include/new/mega.h"
#include "../include/new/move_menu.h"
#include "../include/new/move_menu_battle_scripts.h"
#include "../include/new/move_tables.h"
#include "../include/new/set_z_effect.h"
#include "../include/new/z_move_effects.h"

/*
move_menu.c
	functions for displaying move data and z moves in the battle menu

tables:
	sTypeIconPositions
	sTypeIconPicTable

*/

//TODO: Make The Z-Move Names change colour (look in SetPpNumbersPaletteInMoveSelection)
//Fix Heal Block + Pollen Puff on partner link battle

extern u8 gMoveNames[][MOVE_NAME_LENGTH + 1];
extern u8 gTypeNames[][TYPE_NAME_LENGTH + 1];
extern const u8 sTargetIdentities[];
extern const u16 gUserInterfaceGfx_TypeHighlightingPal[];
extern const u8 gMoveEffectsThatIgnoreWeaknessResistance[];
extern const u8 CamomonsTypeIconsTiles[];
extern const u8 CamomonsTypeIcons2Tiles[];
extern const u16 CamomonsTypeIconsPal[];
extern const u16 CamomonsTypeIcons2Pal[];

//This file's functions:
static bool8 TriggerMegaEvolution(void);
static void MoveSelectionDisplayMoveType(void);
static bool8 MoveSelectionDisplayZMove(void);
static void ZMoveSelectionDisplayPpNumber(void);
static void ZMoveSelectionDisplayPower(void);
static void MoveSelectionDisplayDetails(void);
static void ReloadMoveNamesIfNecessary(void);
static void CloseZMoveDetails(void);
static void TryLoadTypeIcons(void);
static void SpriteCB_CamomonsTypeIcon(struct Sprite* sprite);

static const struct Coords16 sTypeIconPositions[][/*IS_SINGLE_BATTLE*/2] =
{
#ifndef UNBOUND //MODIFY THIS
	[B_POSITION_PLAYER_LEFT] =
	{
		[TRUE] = {221, 86}, 	//Single Battle
		[FALSE] = {144, 70},	//Double Battle
	},
	[B_POSITION_OPPONENT_LEFT] =
	{
		[TRUE] = {20, 26}, 		//Single Battle
		[FALSE] = {97, 14},		//Double Battle
	},
	[B_POSITION_PLAYER_RIGHT] =
	{
		[FALSE] = {156, 96},	//Double Battle
	},
	[B_POSITION_OPPONENT_RIGHT] =
	{
		[FALSE] = {85, 39},		//Double Battle
	},
#else //For Pokemon Unbound
	[B_POSITION_PLAYER_LEFT] =
	{
		[TRUE] = {225, 86}, 	//Single Battle
		[FALSE] = {142, 70},	//Double Battle
	},
	[B_POSITION_OPPONENT_LEFT] =
	{
		[TRUE] = {18, 26}, 		//Single Battle
		[FALSE] = {99, 15},		//Double Battle
	},
	[B_POSITION_PLAYER_RIGHT] =
	{
		[FALSE] = {154, 96},	//Double Battle
	},
	[B_POSITION_OPPONENT_RIGHT] =
	{
		[FALSE] = {87, 40},		//Double Battle
	},
#endif
};

static const struct OamData sTypeIconOAM =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(8x16),
	.size = SPRITE_SIZE(8x16),
	.priority = 1, //Same level as health bar
};

#define type_icon_frame(ptr, frame) {.data = (u8 *)ptr + (1 * 2 * frame * 32), .size = 1 * 2 * 32}
static const struct SpriteFrameImage sTypeIconPicTable[] =
{
	[TYPE_NORMAL] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_NORMAL),
	[TYPE_FIGHTING] =	type_icon_frame(CamomonsTypeIconsTiles, TYPE_FIGHTING),
	[TYPE_FLYING] =		type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_FLYING),
	[TYPE_POISON] =		type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_POISON),
	[TYPE_GROUND] =		type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_GROUND),
	[TYPE_ROCK] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_ROCK),
	[TYPE_BUG] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_BUG),
	[TYPE_GHOST] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_GHOST),
	[TYPE_STEEL] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_STEEL),
	[TYPE_MYSTERY] =	type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_MYSTERY),
	[TYPE_FIRE] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_FIRE),
	[TYPE_WATER] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_WATER),
	[TYPE_GRASS] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_GRASS),
	[TYPE_ELECTRIC] =	type_icon_frame(CamomonsTypeIconsTiles, TYPE_ELECTRIC),
	[TYPE_PSYCHIC] =	type_icon_frame(CamomonsTypeIconsTiles, TYPE_PSYCHIC),
	[TYPE_ICE] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_ICE),
	[TYPE_DRAGON] =		type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_DRAGON),
	[TYPE_DARK] =		type_icon_frame(CamomonsTypeIconsTiles, TYPE_DARK),
	[0x12] =			type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_MYSTERY),
	[TYPE_ROOSTLESS] = 	type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_MYSTERY),
	[TYPE_BLANK] = 		type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_MYSTERY),
	[0x15] = 			type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_MYSTERY),
	[0x16] = 			type_icon_frame(CamomonsTypeIcons2Tiles, TYPE_MYSTERY),
	[TYPE_FAIRY] = 		type_icon_frame(CamomonsTypeIconsTiles, TYPE_FAIRY),
};

static struct SpriteTemplate sTypeIconSpriteTemplate =
{
	.tileTag = 0xFFFF,
	.paletteTag = TYPE_ICON_TAG,
	.oam = &sTypeIconOAM,
	.anims = gDummySpriteAnimTable,
	.images = sTypeIconPicTable,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = SpriteCB_CamomonsTypeIcon,
};

static struct SpriteTemplate sTypeIconSpriteTemplate2 =
{
	.tileTag = 0xFFFF,
	.paletteTag = TYPE_ICON_TAG_2,
	.oam = &sTypeIconOAM,
	.anims = gDummySpriteAnimTable,
	.images = sTypeIconPicTable,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = SpriteCB_CamomonsTypeIcon,
};

static const struct SpritePalette sTypeIconPalTemplate =
{
	.data = CamomonsTypeIconsPal,
	.tag = TYPE_ICON_TAG,
};

static const struct SpritePalette sTypeIconPalTemplate2 =
{
	.data = CamomonsTypeIcons2Pal,
	.tag = TYPE_ICON_TAG_2,
};

void InitMoveSelectionsVarsAndStrings(void)
{
	TryLoadMegaTriggers();
	TryLoadZTrigger();
	TryLoadTypeIcons();
	MoveSelectionDisplayMoveNames();
	gMultiUsePlayerCursor = 0xFF;
	MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0);
	MoveSelectionDisplayPpString();
	MoveSelectionDisplayPpNumber();
	MoveSelectionDisplayMoveType();
}

void HandleInputChooseMove(void)
{
	bool32 canSelectTarget = FALSE;
	struct ChooseMoveStruct* moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);
	gNewBS->ZMoveData->used[gActiveBattler] = moveInfo->zMoveUsed; //So the link opponent's previous Z-selection can't be used
	gNewBS->MegaData->done[gActiveBattler] = moveInfo->megaDone; //So the link opponent's previous Z-selection can't be used
	gNewBS->UltraData->done[gActiveBattler] = moveInfo->ultraDone;

	if (moveInfo->zMoveUsed)
		gNewBS->ZMoveData->toBeUsed[gActiveBattler] = FALSE;

	if (moveInfo->megaDone)
		gNewBS->MegaData->chosen[gActiveBattler] = FALSE;

	if (moveInfo->ultraDone)
		gNewBS->UltraData->chosen[gActiveBattler] = FALSE;

	gNewBS->ZMoveData->partyIndex[SIDE(gActiveBattler)] = moveInfo->zPartyIndex;

	sub_8033AC8();

	if (gMain.newKeys & A_BUTTON)
	{
		u8 moveTarget;
		u16 chosenMove = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];

		PlaySE(SE_SELECT);
		if (chosenMove == MOVE_CURSE)
		{
			if (moveInfo->monType1 != TYPE_GHOST && moveInfo->monType2 != TYPE_GHOST && moveInfo->monType3 != TYPE_GHOST)
				moveTarget = MOVE_TARGET_USER;
			else
				moveTarget = MOVE_TARGET_SELECTED;
		}
		else if (chosenMove == MOVE_ACUPRESSURE && !(IS_DOUBLE_BATTLE))
			moveTarget = MOVE_TARGET_USER; //Only can target yourself in singles
		else
			moveTarget = gBattleMoves[chosenMove].target;

		if (moveTarget & MOVE_TARGET_USER)
			gMultiUsePlayerCursor = gActiveBattler;
		else
			gMultiUsePlayerCursor = GetBattlerAtPosition((GetBattlerPosition(gActiveBattler) & BIT_SIDE) ^ BIT_SIDE);

		if (gBattleBufferA[gActiveBattler][1]) // double battle
		{
			if (gNewBS->ZMoveData->viewing && SPLIT(chosenMove) != SPLIT_STATUS) //Status moves keep original targets
				moveTarget = gBattleMoves[CanUseZMove(gActiveBattler, 0xFF, chosenMove)].target;

			if (!(moveTarget & (MOVE_TARGET_RANDOM | MOVE_TARGET_BOTH | MOVE_TARGET_DEPENDS | MOVE_TARGET_FOES_AND_ALLY | MOVE_TARGET_OPPONENTS_FIELD | MOVE_TARGET_USER)))
				canSelectTarget++; // either selected or user

			if (moveInfo->currentPp[gMoveSelectionCursor[gActiveBattler]] == 0)
			{
				canSelectTarget = FALSE;
			}
			else if (!(moveTarget & (MOVE_TARGET_USER | MOVE_TARGET_USER_OR_PARTNER)) && CountAliveMons(BATTLE_ALIVE_EXCEPT_ACTIVE) <= 1)
			{
				gMultiUsePlayerCursor = GetDefaultMoveTarget(gActiveBattler); //0x803F6A0
				canSelectTarget = FALSE;
			}
		}

		ResetPaletteFadeControl();
		BeginNormalPaletteFade(0xF0000, 0, 0, 0, 0x7FFF);

		if (!canSelectTarget)
		{
			CloseZMoveDetails();
			TryRemoveDoublesKillingScore(gActiveBattler, gMultiUsePlayerCursor, chosenMove);
			EmitMoveChosen(1, gMoveSelectionCursor[gActiveBattler], gMultiUsePlayerCursor, gNewBS->MegaData->chosen[gActiveBattler], gNewBS->UltraData->chosen[gActiveBattler], gNewBS->ZMoveData->toBeUsed[gActiveBattler]);
			PlayerBufferExecCompleted();
		}
		else
		{
			gBattleBankFunc[gActiveBattler] = (u32) HandleInputChooseTarget;

			if (moveTarget & (MOVE_TARGET_USER | MOVE_TARGET_USER_OR_PARTNER))
				gMultiUsePlayerCursor = gActiveBattler;
			else if (gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)])
				gMultiUsePlayerCursor = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
			else
				gMultiUsePlayerCursor = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

			gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
			MoveSelectionDisplayMoveType();
		}
	}
	else if (gMain.newKeys & B_BUTTON)
	{
		if (gNewBS->ZMoveData->viewing) {
			gNewBS->ZMoveData->viewing = FALSE;
			gWindows[3].window.width = 8; //Restore Window Size from Z-Move mod
			gWindows[3].window.height = 2;
			if (gNewBS->ZMoveData->backupTilemap) {
				Free(gWindows[3].tileData);
				gWindows[3].tileData = gNewBS->ZMoveData->backupTilemap;
				gNewBS->ZMoveData->backupTilemap = NULL;
			}
		}

		gNewBS->ZMoveData->viewingDetails = FALSE;
		gMoveSelectionCursor[gActiveBattler] = 0;
		PlaySE(SE_SELECT);
		EmitTwoReturnValues(1, 10, 0xFFFF);
		PlayerBufferExecCompleted();
		ResetPaletteFadeControl();
		BeginNormalPaletteFade(0xF0000, 0, 0, 0, 0x7FFF);
	}
	else if (gMain.newKeys & DPAD_LEFT)
	{
		if (gMoveSelectionCursor[gActiveBattler] & 1)
		{
			ReloadMoveNamesIfNecessary();
			MoveSelectionDestroyCursorAt(gMoveSelectionCursor[gActiveBattler]); //0x8030A14
			gMoveSelectionCursor[gActiveBattler] ^= 1;
			PlaySE(SE_SELECT);
			MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0); //0x80309CC
			MoveSelectionDisplayPpNumber(); //0x80309CC
			MoveSelectionDisplayMoveType();
			BeginNormalPaletteFade(0xF0000, 0, 0, 0, 0x7FFF);
		}
	}
	else if (gMain.newKeys & DPAD_RIGHT)
	{
		if (!(gMoveSelectionCursor[gActiveBattler] & 1)
		 && (gMoveSelectionCursor[gActiveBattler] ^ 1) < gNumberOfMovesToChoose)
		{
			ReloadMoveNamesIfNecessary();
			MoveSelectionDestroyCursorAt(gMoveSelectionCursor[gActiveBattler]);
			gMoveSelectionCursor[gActiveBattler] ^= 1;
			PlaySE(SE_SELECT);
			MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0);
			MoveSelectionDisplayPpNumber();
			MoveSelectionDisplayMoveType();
			BeginNormalPaletteFade(0xF0000, 0, 0, 0, 0x7FFF);
		}
	}
	else if (gMain.newKeys & DPAD_UP)
	{
		if (gMoveSelectionCursor[gActiveBattler] & 2)
		{
			ReloadMoveNamesIfNecessary();
			MoveSelectionDestroyCursorAt(gMoveSelectionCursor[gActiveBattler]);
			gMoveSelectionCursor[gActiveBattler] ^= 2;
			PlaySE(SE_SELECT);
			MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0);
			MoveSelectionDisplayPpNumber();
			MoveSelectionDisplayMoveType();
			BeginNormalPaletteFade(0xF0000, 0, 0, 0, 0x7FFF);
		}
	}
	else if (gMain.newKeys & DPAD_DOWN)
	{
		if (!(gMoveSelectionCursor[gActiveBattler] & 2)
		 && (gMoveSelectionCursor[gActiveBattler] ^ 2) < gNumberOfMovesToChoose)
		{
			ReloadMoveNamesIfNecessary();
			MoveSelectionDestroyCursorAt(gMoveSelectionCursor[gActiveBattler]);
			gMoveSelectionCursor[gActiveBattler] ^= 2;
			PlaySE(SE_SELECT);
			MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0);
			MoveSelectionDisplayPpNumber();
			MoveSelectionDisplayMoveType();
			BeginNormalPaletteFade(0xF0000, 0, 0, 0, 0x7FFF);
		}
	}
	else if (gMain.newKeys & SELECT_BUTTON)
	{
		if (gNumberOfMovesToChoose > 1 && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER)) && !gNewBS->ZMoveData->viewing && !gNewBS->ZMoveData->viewingDetails)
		{
			MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 29);

			if (gMoveSelectionCursor[gActiveBattler] != 0)
				gMultiUsePlayerCursor = 0;
			else
				gMultiUsePlayerCursor = gMoveSelectionCursor[gActiveBattler] + 1;

			MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
			BattlePutTextOnWindow(gText_BattleSwitchWhich, 0xB); //0x80D87BC
			gBattleBankFunc[gActiveBattler] = (u32) HandleMoveSwitching; //0x802EF58
		}
	}
	else if (gMain.newKeys & START_BUTTON)
	{
		if (!MoveSelectionDisplayZMove()) //Only one is allowed at a time
			TriggerMegaEvolution();
	}
	else if (gMain.newKeys & R_BUTTON)
	{
		if (!MoveSelectionDisplayZMove()) //Only one is allowed at a time
			TriggerMegaEvolution();
	}
	else if (gMain.newKeys & L_BUTTON)
	{
		if (!gNewBS->ZMoveData->viewing)
			MoveSelectionDisplayDetails();
	}
}

static bool8 TriggerMegaEvolution(void)
{
	#ifndef MEGA_EVOLUTION_FEATURE
		return FALSE;
	#else
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);
	if (!moveInfo->canMegaEvolve)
		return FALSE;

	if (moveInfo->megaVariance == MEGA_VARIANT_ULTRA_BURST) { //Ultra Burst
		if (gNewBS->UltraData->chosen[gActiveBattler])
		{ // Turn Off
			PlaySE(3);
			gNewBS->UltraData->chosen[gActiveBattler] = FALSE;
			return TRUE;
		}
		else
		{ // Turn On
			PlaySE(2);
			gNewBS->UltraData->chosen[gActiveBattler] = TRUE;
			return TRUE;
		}
	}

	else { //Regular Megas
		if (gNewBS->MegaData->chosen[gActiveBattler]) {
			// Turn Off
			PlaySE(3);
			gNewBS->MegaData->chosen[gActiveBattler] = 0;
			return TRUE;
		}
		else {
			// Turn On
			PlaySE(2);
			gNewBS->MegaData->chosen[gActiveBattler] = 1;
			return TRUE;
		}
	}
	return FALSE;
	#endif
}

//This function sends useful data over Link Cable for the move menu to use
void EmitChooseMove(u8 bufferId, bool8 isDoubleBattle, bool8 NoPpNumber, struct ChooseMoveStruct *movePpData)
{
	u32 i, j;
	const struct Evolution* evolutions;

	struct ChooseMoveStruct* tempMoveStruct = Calloc(sizeof(struct ChooseMoveStruct)); //Make space for new expanded data
	Memcpy(tempMoveStruct, movePpData, sizeof(struct ChooseMoveStructOld)); //Copy the old data
	tempMoveStruct->monType1 = gBattleMons[gActiveBattler].type1;
	tempMoveStruct->monType2 = gBattleMons[gActiveBattler].type2;
	tempMoveStruct->monType3 = gBattleMons[gActiveBattler].type3;
	tempMoveStruct->ability = ABILITY(gActiveBattler);

	gBattleScripting->dmgMultiplier = 1;
	for (i = 0; i < MAX_MON_MOVES; ++i)
	{
		u8 foe = (IS_DOUBLE_BATTLE && !BATTLER_ALIVE(FOE(gActiveBattler))) ? PARTNER(FOE(gActiveBattler)) : FOE(gActiveBattler);
		u16 move = gBattleMons[gActiveBattler].moves[i];

		tempMoveStruct->moves[i] = move;
		tempMoveStruct->moveTypes[i] = GetMoveTypeSpecial(gActiveBattler, move);

		if (IS_DOUBLE_BATTLE && CountAliveMonsInBattle(BATTLE_ALIVE_DEF_SIDE, gActiveBattler, foe) >= 2) //Because target can vary, display only attacker's modifiers
		{
			tempMoveStruct->movePowers[i] = GetBasePower(gActiveBattler, foe, move,
									 gBattleMons[gActiveBattler].item, ITEM_EFFECT(gActiveBattler), ABILITY(gActiveBattler),
									 gBattleMons[gActiveBattler].status1, gBattleMons[gActiveBattler].hp, gBattleMons[gActiveBattler].maxHP,
									 gBattleMons[gActiveBattler].species, GetBankPartyData(gActiveBattler), FALSE, TRUE, TRUE);
			tempMoveStruct->movePowers[i] = CalcVisualBasePower(gActiveBattler, gActiveBattler,
																move, tempMoveStruct->movePowers[i],
																tempMoveStruct->moveTypes[i], TRUE);
			tempMoveStruct->moveAcc[i] = VisualAccuracyCalc_NoTarget(move, gActiveBattler);

			for (j = 0; j < gBattlersCount; ++j)
			{
				if (SPLIT(move) != SPLIT_STATUS)
				{
					u8 moveResult;

					if (j == gActiveBattler || j == PARTNER(gActiveBattler))
					{
						tempMoveStruct->moveResults[GetBattlerPosition(j)][i] = 0;
						continue;
					}

					moveResult = VisualTypeCalc(move, gActiveBattler, j);

					if (!(moveResult & MOVE_RESULT_NO_EFFECT) && CheckTableForMoveEffect(move, gMoveEffectsThatIgnoreWeaknessResistance))
						moveResult = 0; //These moves can have no effect, but are neither super nor not very effective
					tempMoveStruct->moveResults[GetBattlerPosition(j)][i] = moveResult;
				}
				else
					tempMoveStruct->moveResults[GetBattlerPosition(j)][i] = 0;
			}
		}
		else
		{
			tempMoveStruct->movePowers[i] = GetBasePower(gActiveBattler, foe, move,
									 gBattleMons[gActiveBattler].item, ITEM_EFFECT(gActiveBattler), ABILITY(gActiveBattler),
									 gBattleMons[gActiveBattler].status1, gBattleMons[gActiveBattler].hp, gBattleMons[gActiveBattler].maxHP,
									 gBattleMons[gActiveBattler].species, GetBankPartyData(gActiveBattler), FALSE, TRUE, FALSE);
			tempMoveStruct->movePowers[i] = CalcVisualBasePower(gActiveBattler, foe,
																move, tempMoveStruct->movePowers[i],
																tempMoveStruct->moveTypes[i], FALSE);
			tempMoveStruct->moveAcc[i] = VisualAccuracyCalc(move, gActiveBattler, foe);

			if (SPLIT(move) != SPLIT_STATUS)
			{
				u8 moveResult = VisualTypeCalc(move, gActiveBattler, foe);

				if (!(moveResult & MOVE_RESULT_NO_EFFECT) && CheckTableForMoveEffect(move, gMoveEffectsThatIgnoreWeaknessResistance))
					moveResult = 0; //These moves can have no effect, but are neither super nor not very effective

				tempMoveStruct->moveResults[GetBattlerPosition(foe)][i] = moveResult;
			}
			else
				tempMoveStruct->moveResults[GetBattlerPosition(foe)][i] = 0;
		}
	}

	tempMoveStruct->megaDone = gNewBS->MegaData->done[gActiveBattler];
	tempMoveStruct->ultraDone = gNewBS->UltraData->done[gActiveBattler];
	if (!IS_TRANSFORMED(gActiveBattler))
	{
		if (!gNewBS->MegaData->done[gActiveBattler])
		{
			evolutions = CanMegaEvolve(gActiveBattler, FALSE);
			if (evolutions != NULL)
			{
				if (!BankMegaEvolved(gActiveBattler, FALSE)
				&& MegaEvolutionEnabled(gActiveBattler)
				&& !DoesZMoveUsageStopMegaEvolution(gActiveBattler)) //No Mega Evolving if you've used a Z-Move (*cough* *cough* Rayquaza)
				{
					tempMoveStruct->canMegaEvolve = TRUE;
					tempMoveStruct->megaVariance = evolutions->unknown;
				}
			}
		}

		if (!gNewBS->UltraData->done[gActiveBattler])
		{
			evolutions = CanMegaEvolve(gActiveBattler, TRUE); //Check Ultra Burst
			if (evolutions != NULL)
			{
				if (!BankMegaEvolved(gActiveBattler, TRUE)) //Check Ultra Burst
				{
					tempMoveStruct->canMegaEvolve = TRUE;
					tempMoveStruct->megaVariance = evolutions->unknown;
				}
			}
		}
	}

	tempMoveStruct->zMoveUsed = gNewBS->ZMoveData->used[gActiveBattler];
	tempMoveStruct->zPartyIndex = gNewBS->ZMoveData->partyIndex[SIDE(gActiveBattler)];

	if (!gNewBS->ZMoveData->used[gActiveBattler] && !IsMega(gActiveBattler) && !IsBluePrimal(gActiveBattler) && !IsRedPrimal(gActiveBattler))
	{
		u8 limitations = CheckMoveLimitations(gActiveBattler, 0, MOVE_LIMITATION_PP);

		for (i = 0; i < MAX_MON_MOVES; ++i)
		{
			if (!(limitations & gBitTable[i])) //Don't display a Z-Move if the base move has no PP
				tempMoveStruct->possibleZMoves[i] = CanUseZMove(gActiveBattler, i, 0);
		}
	}

	gBattleBuffersTransferData[0] = CONTROLLER_CHOOSEMOVE;
	gBattleBuffersTransferData[1] = isDoubleBattle;
	gBattleBuffersTransferData[2] = NoPpNumber;
	gBattleBuffersTransferData[3] = 0;
	for (i = 0; i < sizeof(*tempMoveStruct); ++i)
		gBattleBuffersTransferData[4 + i] = ((u8*)(tempMoveStruct))[i];
	PrepareBufferDataTransfer(bufferId, gBattleBuffersTransferData, sizeof(*tempMoveStruct) + 4);

	Free(tempMoveStruct);
}

void EmitMoveChosen(u8 bufferId, u8 chosenMoveIndex, u8 target, u8 megaState, u8 ultraState, u8 zMoveState)
{
	gBattleBuffersTransferData[0] = CONTROLLER_TWORETURNVALUES;
	gBattleBuffersTransferData[1] = ACTION_RUN_BATTLESCRIPT;
	gBattleBuffersTransferData[2] = chosenMoveIndex;
	gBattleBuffersTransferData[3] = target;
	gBattleBuffersTransferData[4] = megaState;
	gBattleBuffersTransferData[5] = ultraState;
	gBattleBuffersTransferData[6] = zMoveState;
	PrepareBufferDataTransfer(bufferId, gBattleBuffersTransferData, 7);
}

const u8 sText_StabMoveInterfaceType[] = {0xFC, 0x05, 0x05, 0xFC, 0x04, 0x09, 0x0E, 0x08, 0xFF};

#define SUPER_EFFECTIVE_COLOURS 0
#define NOT_VERY_EFFECTIVE_COLOURS 4
#define NO_EFFECT_COLOURS 8
#define REGULAR_COLOURS 12
static void MoveSelectionDisplayMoveType(void)
{
	u8 *txtPtr;
	const u8* formatting;
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);

	#ifdef DISPLAY_REAL_MOVE_TYPE_ON_MENU
		u8 moveType = moveInfo->moveTypes[gMoveSelectionCursor[gActiveBattler]];
	#else
		u8 moveType = gBattleMoves[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]].type;
	#endif

	//Update Palette Fading for Effectiveness
	#ifdef DISPLAY_EFFECTIVENESS_ON_MENU
		if (!(gBattleTypeFlags & BATTLE_TYPE_LINK) && !IS_GHOST_BATTLE) //Don't use this feature in link battles or battles against Ghosts
		{
			u8 stab = 0;
			const u16* palPtr = gUserInterfaceGfx_TypeHighlightingPal;
			if (SPLIT(moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]) != SPLIT_STATUS
			&&	(moveType == moveInfo->monType1
			  || moveType == moveInfo->monType2
			  || moveType == moveInfo->monType3))
			{
				stab = 2;
			}

			u8 moveResult = 0;
			if (IS_SINGLE_BATTLE)
				moveResult = moveInfo->moveResults[GetBattlerPosition(FOE(gActiveBattler))][gMoveSelectionCursor[gActiveBattler]];
			else if (gBattleBankFunc[gActiveBattler] == (u32) HandleInputChooseTarget)
				moveResult = moveInfo->moveResults[GetBattlerPosition(gMultiUsePlayerCursor)][gMoveSelectionCursor[gActiveBattler]];

			if (moveResult & MOVE_RESULT_SUPER_EFFECTIVE)
			{
				gPlttBufferUnfaded[88] = palPtr[SUPER_EFFECTIVE_COLOURS + stab + 0];
				gPlttBufferUnfaded[89] = palPtr[SUPER_EFFECTIVE_COLOURS + stab + 1];
				formatting = sText_StabMoveInterfaceType;
			}
			else if (moveResult & MOVE_RESULT_NOT_VERY_EFFECTIVE)
			{
				gPlttBufferUnfaded[88] = palPtr[NOT_VERY_EFFECTIVE_COLOURS + stab + 0];
				gPlttBufferUnfaded[89] = palPtr[NOT_VERY_EFFECTIVE_COLOURS + stab + 1];
				formatting = sText_StabMoveInterfaceType;
			}
			else if (moveResult & MOVE_RESULT_NO_EFFECT)
			{
				gPlttBufferUnfaded[88] = palPtr[NO_EFFECT_COLOURS + 0]; //No STAB for moves with no effect
				gPlttBufferUnfaded[89] = palPtr[NO_EFFECT_COLOURS + 1];
				formatting = sText_StabMoveInterfaceType;
			}
			else //Nothing special about move
			{
				gPlttBufferUnfaded[88] = palPtr[REGULAR_COLOURS + stab + 0];
				gPlttBufferUnfaded[89] = palPtr[REGULAR_COLOURS + stab + 1];
				formatting = sText_StabMoveInterfaceType;
			}
		}
		else
	#endif
			formatting = gText_MoveInterfaceType;

	CpuCopy16(&gPlttBufferUnfaded[88], &gPlttBufferFaded[88], sizeof(u16));
	CpuCopy16(&gPlttBufferUnfaded[89], &gPlttBufferFaded[89], sizeof(u16));

	txtPtr = StringCopy(gDisplayedStringBattle, gText_TypeWord);
	txtPtr[0] = EXT_CTRL_CODE_BEGIN;
	txtPtr++;
	txtPtr[0] = 6;
	txtPtr++;
	txtPtr[0] = 1;
	txtPtr++;
	txtPtr = StringCopy(txtPtr, formatting);

	StringCopy(txtPtr, gTypeNames[moveType]);
	BattlePutTextOnWindow(gDisplayedStringBattle, 8);
}

static bool8 MoveSelectionDisplayZMove(void)
{
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);
	u16 zmove = moveInfo->possibleZMoves[gMoveSelectionCursor[gActiveBattler]];
	u8 i;

	if (gNewBS->ZMoveData->viewing)
	{
		ReloadMoveNamesIfNecessary();
		PlaySE(3); //Turn Off
		return TRUE;
	}

	if (zmove != MOVE_NONE)
	{
		for (i = 0; i < MAX_MON_MOVES; ++i)
		{
			MoveSelectionDestroyCursorAt(i);
			StringCopy(gDisplayedStringBattle, StringNull);
			BattlePutTextOnWindow(gDisplayedStringBattle, i + 3);
		}

		if (zmove == 0xFFFF)
		{
			u8 zEffect = gBattleMoves[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]].z_move_effect;

			if (zEffect == Z_EFFECT_CURSE)
			{
				if (moveInfo->monType1 == TYPE_GHOST || moveInfo->monType2 == TYPE_GHOST || moveInfo->monType3 == TYPE_GHOST)
					zEffect = Z_EFFECT_RECOVER_HP;
				else
					zEffect = Z_EFFECT_ATK_UP_1;
			}

			gDisplayedStringBattle[0] = EOS;
			switch (zEffect) {
				case Z_EFFECT_RESET_STATS:
					StringCopy(gDisplayedStringBattle, gText_ResetStats);
					break;

				case Z_EFFECT_ALL_STATS_UP_1:
					StringCopy(gDisplayedStringBattle, gText_StatsPlus);
					break;

				case Z_EFFECT_BOOST_CRITS:
					StringCopy(gDisplayedStringBattle, gText_CritHitsPlus);
					break;

				case Z_EFFECT_FOLLOW_ME:
					StringCopy(gDisplayedStringBattle, gText_FollowMe);
					break;

				case Z_EFFECT_RECOVER_HP:
					StringCopy(gDisplayedStringBattle, gText_RecoverHP);
					break;

				case Z_EFFECT_RESTORE_REPLACEMENT_HP:
					StringCopy(gDisplayedStringBattle, gText_HealAllyHP);
					break;

				case Z_EFFECT_ATK_UP_1:
				case Z_EFFECT_DEF_UP_1:
				case Z_EFFECT_SPD_UP_1:
				case Z_EFFECT_SPATK_UP_1:
				case Z_EFFECT_SPDEF_UP_1:
				case Z_EFFECT_ACC_UP_1:
				case Z_EFFECT_EVSN_UP_1:
					gDisplayedStringBattle[0] = TXT_PLUS;
					gDisplayedStringBattle[1] = 0;
					gDisplayedStringBattle[2] = EOS;
					PREPARE_STAT_BUFFER(gBattleTextBuff1, zEffect - Z_EFFECT_ATK_UP_1 + 1);
					ExpandBattleTextBuffPlaceholders(gBattleTextBuff1, gDisplayedStringBattle + 2);
					break;

				case Z_EFFECT_ATK_UP_2:
				case Z_EFFECT_DEF_UP_2:
				case Z_EFFECT_SPD_UP_2:
				case Z_EFFECT_SPATK_UP_2:
				case Z_EFFECT_SPDEF_UP_2:
				case Z_EFFECT_ACC_UP_2:
				case Z_EFFECT_EVSN_UP_2:
					gDisplayedStringBattle[0] = TXT_PLUS;
					gDisplayedStringBattle[1] = TXT_PLUS;
					gDisplayedStringBattle[2] = 0;
					gDisplayedStringBattle[3] = EOS;
					PREPARE_STAT_BUFFER(gBattleTextBuff1, zEffect - Z_EFFECT_ATK_UP_2 + 1);
					ExpandBattleTextBuffPlaceholders(gBattleTextBuff1, gDisplayedStringBattle + 3);
					break;

				case Z_EFFECT_ATK_UP_3:
				case Z_EFFECT_DEF_UP_3:
				case Z_EFFECT_SPD_UP_3:
				case Z_EFFECT_SPATK_UP_3:
				case Z_EFFECT_SPDEF_UP_3:
				case Z_EFFECT_ACC_UP_3:
				case Z_EFFECT_EVSN_UP_3:
					gDisplayedStringBattle[0] = TXT_PLUS;
					gDisplayedStringBattle[1] = TXT_PLUS;
					gDisplayedStringBattle[2] = TXT_PLUS;
					gDisplayedStringBattle[3] = 0;
					gDisplayedStringBattle[4] = EOS;
					PREPARE_STAT_BUFFER(gBattleTextBuff1, zEffect - Z_EFFECT_ATK_UP_3 + 1);
					ExpandBattleTextBuffPlaceholders(gBattleTextBuff1, gDisplayedStringBattle + 4);
					break;
			}
			BattlePutTextOnWindow(gDisplayedStringBattle, 3 + 2); //Slot of Move 3

			gDisplayedStringBattle[0] = 0xD4;
			gDisplayedStringBattle[1] = 0xAE; //Z-
			StringCopy(gDisplayedStringBattle + 2, gMoveNames[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]]);
			gWindows[3].window.width = 16;
		}
		else
		{
			gWindows[3].window.width = 17;
			gWindows[3].window.height = 4; //Double the size to help super long Z-move names
			ZMoveSelectionDisplayPower();
			StringCopy(gDisplayedStringBattle, GetZMoveName(zmove));
		}
		gNewBS->ZMoveData->backupTilemap = gWindows[3].tileData;
		gWindows[3].tileData = Calloc(0x880); //Because of the size expansion of the tile map, new memory was necessary to display the Z-Move name
		BattlePutTextOnWindow(gDisplayedStringBattle, 3);

		ZMoveSelectionDisplayPpNumber();
		MoveSelectionCreateCursorAt(0, 0);
		gNewBS->ZMoveData->viewing = TRUE;
		PlaySE(2); //Turn On
		return TRUE;
	}

	return FALSE;
}

static void ZMoveSelectionDisplayPpNumber(void)
{
	u8 *txtPtr;
	const u16* palPtr = Pal_PPDisplay;

	//Remove Palette Fading On The PP
	gPlttBufferUnfaded[92] = palPtr[(3 * 2) + 0];
	gPlttBufferUnfaded[91] = palPtr[(3 * 2) + 1];

	CpuCopy16(&gPlttBufferUnfaded[92], &gPlttBufferFaded[92], sizeof(u16));
	CpuCopy16(&gPlttBufferUnfaded[91], &gPlttBufferFaded[91], sizeof(u16));

	//Load PP Text
	txtPtr = ConvertIntToDecimalStringN(gDisplayedStringBattle, 1, STR_CONV_MODE_RIGHT_ALIGN, 2);
	txtPtr[0] = CHAR_SLASH;
	txtPtr++;
	ConvertIntToDecimalStringN(txtPtr, 1, STR_CONV_MODE_RIGHT_ALIGN, 2);

	BattlePutTextOnWindow(gDisplayedStringBattle, 9);
}

static void ZMoveSelectionDisplayPower(void)
{
	u8 *txtPtr;
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);
	u16 move = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
	u8 power = gBattleMoves[move].z_move_power;

	u16 zmove = moveInfo->possibleZMoves[gMoveSelectionCursor[gActiveBattler]];

	if (zmove >= MOVE_CATASTROPIKA)
		power = gBattleMoves[zmove].power;

	if (SPLIT(move) != SPLIT_STATUS) {
	/* 	The preffered way was to display the power string under the Z-Move name.
		However, it causes too many graphical issues that I don't know how to fix.

		txtPtr = StringCopy(gDisplayedStringBattle, gText_Power);
		ConvertIntToDecimalStringN(txtPtr, power, STR_CONV_MODE_LEFT_ALIGN, 3);
		BattlePutTextOnWindow(gDisplayedStringBattle, 3 + 3); //Slot of Move 3
	*/
		txtPtr = StringCopy(gDisplayedStringBattle, gText_Power);
		txtPtr[0] = EXT_CTRL_CODE_BEGIN;
		txtPtr++;
		txtPtr[0] = 6;
		txtPtr++;
		txtPtr[0] = 1;
		txtPtr++;
		ConvertIntToDecimalStringN(txtPtr, power, STR_CONV_MODE_LEFT_ALIGN, 3);
		BattlePutTextOnWindow(gDisplayedStringBattle, 8); //Where Move Type goes
	}
}

static void MoveSelectionDisplayDetails(void)
{
	u8 *txtPtr;
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);

	if (gNewBS->ZMoveData->viewingDetails)
	{
		ReloadMoveNamesIfNecessary();
		return;
	}

	for (int i = 0; i < MAX_MON_MOVES; ++i)
	{
		MoveSelectionDestroyCursorAt(i);
		StringCopy(gDisplayedStringBattle, StringNull);
		BattlePutTextOnWindow(gDisplayedStringBattle, i + 3);
	}

//Diaplay Move Name
	StringCopy(gDisplayedStringBattle, gMoveNames[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]]);
	BattlePutTextOnWindow(gDisplayedStringBattle, 3);

//Display Move Accuracy
	txtPtr = StringCopy(gDisplayedStringBattle, gText_Acc);
	#ifdef DISPLAY_REAL_ACCURACY_ON_MENU
		if (moveInfo->moveAcc[gMoveSelectionCursor[gActiveBattler]] == 0xFFFF)
			StringCopy(txtPtr, gText_NoMiss);
		else
			ConvertIntToDecimalStringN(txtPtr, moveInfo->moveAcc[gMoveSelectionCursor[gActiveBattler]], STR_CONV_MODE_LEFT_ALIGN, 3);
	#else
		if (gBattleMoves[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]].accuracy == 0)
			StringCopy(txtPtr, gText_NoMiss);
		else
			ConvertIntToDecimalStringN(txtPtr, gBattleMoves[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]].accuracy, STR_CONV_MODE_LEFT_ALIGN, 3);
	#endif
	BattlePutTextOnWindow(gDisplayedStringBattle, 3 + 1); //Slot of Move 2

//Display Move Power
	txtPtr = StringCopy(gDisplayedStringBattle, gText_Power);
	#ifdef DISPLAY_REAL_POWER_ON_MENU
		ConvertIntToDecimalStringN(txtPtr, moveInfo->movePowers[gMoveSelectionCursor[gActiveBattler]], STR_CONV_MODE_LEFT_ALIGN, 3);
	#else
		ConvertIntToDecimalStringN(txtPtr, gBattleMoves[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]].power, STR_CONV_MODE_LEFT_ALIGN, 3);
	#endif
	BattlePutTextOnWindow(gDisplayedStringBattle, 3 + 2); //Slot of Move 3

//Display Move Split
	switch(SPLIT(moveInfo->moves[gMoveSelectionCursor[gActiveBattler]])) {
		case SPLIT_SPECIAL:
			StringCopy(gDisplayedStringBattle, gText_Special);
			break;
		case SPLIT_STATUS:
			StringCopy(gDisplayedStringBattle, gText_Status);
			break;
		default:
			StringCopy(gDisplayedStringBattle, gText_Physical);
	}
	BattlePutTextOnWindow(gDisplayedStringBattle, 3 + 3); //Slot of Move 4

	MoveSelectionCreateCursorAt(0, 0);
	gNewBS->ZMoveData->viewingDetails = TRUE;
}

static void ReloadMoveNamesIfNecessary(void)
{
	if (gNewBS->ZMoveData->viewing)
	{
		gNewBS->ZMoveData->viewing = FALSE;
		gWindows[3].window.width = 8; //Restore Window Size from Z-Move mod
		gWindows[3].window.height = 2;

		if (gNewBS->ZMoveData->backupTilemap)
		{
			Free(gWindows[3].tileData);
			gWindows[3].tileData = gNewBS->ZMoveData->backupTilemap;
			gNewBS->ZMoveData->backupTilemap = NULL;
		}

		MoveSelectionDestroyCursorAt(0);
		MoveSelectionDisplayMoveNames();
		MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0);
		MoveSelectionDisplayPpNumber();
		MoveSelectionDisplayMoveType();
	}
	else if (gNewBS->ZMoveData->viewingDetails)
	{
		gNewBS->ZMoveData->viewingDetails = FALSE;
		MoveSelectionDestroyCursorAt(0);
		MoveSelectionDisplayMoveNames();
		MoveSelectionCreateCursorAt(gMoveSelectionCursor[gActiveBattler], 0);
		MoveSelectionDisplayPpNumber();
		MoveSelectionDisplayMoveType();
	}
}

static void CloseZMoveDetails(void)
{
	if (gNewBS->ZMoveData->viewing)
	{
		gNewBS->ZMoveData->toBeUsed[gActiveBattler] = TRUE;
		gNewBS->ZMoveData->viewing = FALSE;
		gWindows[3].window.width = 8; //Restore Window Size from Z-Move mod
		gWindows[3].window.height = 2;
		if (gNewBS->ZMoveData->backupTilemap)
		{
			Free(gWindows[3].tileData);
			gWindows[3].tileData = gNewBS->ZMoveData->backupTilemap;
			gNewBS->ZMoveData->backupTilemap = NULL;
		}
	}

	gNewBS->ZMoveData->viewingDetails = FALSE;
}

void HandleMoveSwitchingUpdate(void)
{
	u8 perMovePPBonuses[4];
	struct ChooseMoveStruct moveStruct;
	u8 totalPPBonuses;
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);
	int i;

	//Swap Moves
	i = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->moves[gMoveSelectionCursor[gActiveBattler]] = moveInfo->moves[gMultiUsePlayerCursor];
	moveInfo->moves[gMultiUsePlayerCursor] = i;

	//Swap PP
	i = moveInfo->currentPp[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->currentPp[gMoveSelectionCursor[gActiveBattler]] = moveInfo->currentPp[gMultiUsePlayerCursor];
	moveInfo->currentPp[gMultiUsePlayerCursor] = i;

	i = moveInfo->maxPp[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->maxPp[gMoveSelectionCursor[gActiveBattler]] = moveInfo->maxPp[gMultiUsePlayerCursor];
	moveInfo->maxPp[gMultiUsePlayerCursor] = i;

	//Swap Move Types Details
	i = moveInfo->moveTypes[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->moveTypes[gMoveSelectionCursor[gActiveBattler]] = moveInfo->moveTypes[gMultiUsePlayerCursor];
	moveInfo->moveTypes[gMultiUsePlayerCursor] = i;

	//Swap Move Powers Details
	i = moveInfo->movePowers[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->movePowers[gMoveSelectionCursor[gActiveBattler]] = moveInfo->movePowers[gMultiUsePlayerCursor];
	moveInfo->movePowers[gMultiUsePlayerCursor] = i;

	//Swap Move Accuracies Details
	i = moveInfo->moveAcc[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->moveAcc[gMoveSelectionCursor[gActiveBattler]] = moveInfo->moveAcc[gMultiUsePlayerCursor];
	moveInfo->moveAcc[gMultiUsePlayerCursor] = i;

	//Swap Possible Z-Moves
	i = moveInfo->possibleZMoves[gMoveSelectionCursor[gActiveBattler]];
	moveInfo->possibleZMoves[gMoveSelectionCursor[gActiveBattler]] = moveInfo->possibleZMoves[gMultiUsePlayerCursor];
	moveInfo->possibleZMoves[gMultiUsePlayerCursor] = i;

	if (gDisableStructs[gActiveBattler].mimickedMoves & gBitTable[gMoveSelectionCursor[gActiveBattler]])
	{
		gDisableStructs[gActiveBattler].mimickedMoves &= (~gBitTable[gMoveSelectionCursor[gActiveBattler]]);
		gDisableStructs[gActiveBattler].mimickedMoves |= gBitTable[gMultiUsePlayerCursor];
	}

	MoveSelectionDisplayMoveNames();

	for (i = 0; i < MAX_MON_MOVES; i++)
		perMovePPBonuses[i] = (gBattleMons[gActiveBattler].ppBonuses & (3 << (i * 2))) >> (i * 2);

	totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]];
	perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]] = perMovePPBonuses[gMultiUsePlayerCursor];
	perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;

	totalPPBonuses = 0;
	for (i = 0; i < MAX_MON_MOVES; i++)
		totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

	gBattleMons[gActiveBattler].ppBonuses = totalPPBonuses;

	for (i = 0; i < MAX_MON_MOVES; i++)
	{
		gBattleMons[gActiveBattler].moves[i] = moveInfo->moves[i];
		gBattleMons[gActiveBattler].pp[i] = moveInfo->currentPp[i];
	}

	if (!(gBattleMons[gActiveBattler].status2 & STATUS2_TRANSFORMED))
	{
		for (i = 0; i < MAX_MON_MOVES; i++)
		{
			moveStruct.moves[i] = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MOVE1 + i, 0);
			moveStruct.currentPp[i] = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP1 + i, 0);
		}

		totalPPBonuses = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP_BONUSES, 0);
		for (i = 0; i < MAX_MON_MOVES; i++)
			perMovePPBonuses[i] = (totalPPBonuses & (3 << (i * 2))) >> (i * 2);

		i = moveStruct.moves[gMoveSelectionCursor[gActiveBattler]];
		moveStruct.moves[gMoveSelectionCursor[gActiveBattler]] = moveStruct.moves[gMultiUsePlayerCursor];
		moveStruct.moves[gMultiUsePlayerCursor] = i;

		i = moveStruct.currentPp[gMoveSelectionCursor[gActiveBattler]];
		moveStruct.currentPp[gMoveSelectionCursor[gActiveBattler]] = moveStruct.currentPp[gMultiUsePlayerCursor];
		moveStruct.currentPp[gMultiUsePlayerCursor] = i;

		totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]];
		perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]] = perMovePPBonuses[gMultiUsePlayerCursor];
		perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;

		totalPPBonuses = 0;
		for (i = 0; i < MAX_MON_MOVES; i++)
			totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

		for (i = 0; i < MAX_MON_MOVES; i++)
		{
			SetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MOVE1 + i, &moveStruct.moves[i]);
			SetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP1 + i, &moveStruct.currentPp[i]);
		}

		SetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP_BONUSES, &totalPPBonuses);
	}
}

void HandleInputChooseTarget(void)
{
	int i;
	u8 identities[4];
	memcpy(identities, sTargetIdentities, 4 /*ARRAY_COUNT(sTargetIdentities)*/); //sTargetIdentities = 0x8250980
	struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleBufferA[gActiveBattler][4]);

	DoBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX, 15, 1); //0x8012138

	if (gBattlersCount != 0)
	{
		for (i = 0; i < gBattlersCount; ++i)
		{
			if (i != gMultiUsePlayerCursor)
				EndBounceEffect(i, BOUNCE_HEALTHBOX); //0x8012258
		}
	}

	if (gMain.newKeys & A_BUTTON)
	{
		PlaySE(SE_SELECT);
		gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012098; //sub_8039B2C in Emerald
		TryRemoveDoublesKillingScore(gActiveBattler, gMultiUsePlayerCursor, moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]);
		EmitMoveChosen(1, gMoveSelectionCursor[gActiveBattler], gMultiUsePlayerCursor, gNewBS->MegaData->chosen[gActiveBattler], gNewBS->UltraData->chosen[gActiveBattler], gNewBS->ZMoveData->toBeUsed[gActiveBattler]);
		CloseZMoveDetails();
		EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
		PlayerBufferExecCompleted();
	}
	else if (gMain.newKeys & B_BUTTON)
	{
		PlaySE(SE_SELECT);
		gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012098; //sub_8039B2C in Emerald
		gBattleBankFunc[gActiveBattler] = (u32) HandleInputChooseMove;
		DoBounceEffect(gActiveBattler, BOUNCE_HEALTHBOX, 7, 1);
		DoBounceEffect(gActiveBattler, BOUNCE_MON, 7, 1);
		EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);

		ReloadMoveNamesIfNecessary();
		TryLoadMegaTriggers();
		TryLoadZTrigger();
		MoveSelectionDisplayMoveType();
	}
	else if (gMain.newKeys & (DPAD_LEFT | DPAD_UP))
	{
		PlaySE(SE_SELECT);
		gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012098; //sub_8039B2C in Emerald

		do
		{
			u8 currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

			u16 move = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
			switch (move) {
				case MOVE_ACUPRESSURE:
					if (!(gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor ^ BIT_FLANK]))
					{
						gMultiUsePlayerCursor ^= BIT_FLANK;
					}
					gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
					return;

				case MOVE_POLLENPUFF:
					if (IsHealBlocked(gActiveBattler) //Affected by Heal Block can't target partner
					&& !(gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor ^ 2]))
					{
						gMultiUsePlayerCursor ^= BIT_FLANK;
						gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
						return;
					}
			}

			for (i = 0; i < MAX_BATTLERS_COUNT; i++)
			{
				if (currSelIdentity == identities[i])
					break;
			}
			do
			{
				if (--i < 0)
					i = MAX_BATTLERS_COUNT - 1; // UB: array out of range
				gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
			} while (gMultiUsePlayerCursor == gBattlersCount);

			i = 0;
			move = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
			switch (GetBattlerPosition(gMultiUsePlayerCursor)) {
				case B_POSITION_PLAYER_LEFT:
				case B_POSITION_PLAYER_RIGHT:
					if (gActiveBattler != gMultiUsePlayerCursor)
						i++;
					else if (gBattleMoves[move].target & MOVE_TARGET_USER_OR_PARTNER)
						i++;
					break;
				case B_POSITION_OPPONENT_LEFT:
				case B_POSITION_OPPONENT_RIGHT:
					i++;
					break;
			}

			if (gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor])
				i = 0;

		} while (i == 0);
		gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
		MoveSelectionDisplayMoveType();
	}
	else if (gMain.newKeys & (DPAD_RIGHT | DPAD_DOWN))
	{
		PlaySE(SE_SELECT);
		gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012098; //sub_8039B2C in Emerald

		do
		{
			u8 currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

			u16 move = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
			switch (move) {
				case MOVE_ACUPRESSURE:
					if (!(gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor ^ BIT_FLANK]))
					{
						gMultiUsePlayerCursor ^= BIT_FLANK;
					}
					gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
					return;

				case MOVE_POLLENPUFF:
					if (IsHealBlocked(gActiveBattler) //Affected by Heal Block can't target partner
					&& !(gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor ^ 2]))
					{
						gMultiUsePlayerCursor ^= BIT_FLANK;
						gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
						return;
					}
			}

			for (i = 0; i < MAX_BATTLERS_COUNT; i++)
			{
				if (currSelIdentity == identities[i])
					break;
			}
			do
			{
				if (++i > 3)
					i = 0;
				gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
			} while (gMultiUsePlayerCursor == gBattlersCount);

			i = 0;
			move = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
			switch (GetBattlerPosition(gMultiUsePlayerCursor)) {
				case B_POSITION_PLAYER_LEFT:
				case B_POSITION_PLAYER_RIGHT:
					if (gActiveBattler != gMultiUsePlayerCursor)
						i++;
					else if (gBattleMoves[move].target & MOVE_TARGET_USER_OR_PARTNER)
						i++;
					break;
				case B_POSITION_OPPONENT_LEFT:
				case B_POSITION_OPPONENT_RIGHT:
					i++;
					break;
			}

			if (gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor])
				i = 0;

		} while (i == 0);
		gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = sub_8012044; //sub_8039AD8 in Emerald
		MoveSelectionDisplayMoveType();
	}
}

u8 TrySetCantSelectMoveBattleScript(void)
{
	u8 limitations = 0;
	u16 move = gBattleMons[gActiveBattler].moves[gBattleBufferB[gActiveBattler][2]];
	u16* choicedMove = &gBattleStruct->choicedMove[gActiveBattler];
	u8 holdEffect = ITEM_EFFECT(gActiveBattler);
	gStringBank = gActiveBattler;
	gBattleScripting->bank = gActiveBattler;
	gCurrentMove = move;

	if (gDisableStructs[gActiveBattler].encoredMove
	&& gDisableStructs[gActiveBattler].encoredMove != move
	&& !gNewBS->ZMoveData->toBeUsed[gActiveBattler]) //Can still use other Z-Moves even if encored
	{
		gCurrentMove = gDisableStructs[gActiveBattler].encoredMove;
		gSelectionBattleScripts[gActiveBattler] = BattleScript_MustSelectEncoredMove;
		++limitations;
	}
	else if (gDisableStructs[gActiveBattler].disabledMove == move && move != MOVE_NONE)
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingDisabledMove;
		++limitations;
	}
	else if (move == gLastUsedMoves[gActiveBattler] && move != MOVE_STRUGGLE && IsTormented(gActiveBattler))
	{
		CancelMultiTurnMoves(gActiveBattler);
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingTormentedMove;
		++limitations;
	}
	else if (IsTaunted(gActiveBattler) != 0 && SPLIT(move) == SPLIT_STATUS)
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedMoveTaunt;
		limitations++;
	}
	else if (IsImprisoned(gActiveBattler, move))
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingImprisionedMove;
		++limitations;
	}
	else if (holdEffect == ITEM_EFFECT_CHOICE_BAND && *choicedMove != 0 && *choicedMove != 0xFFFF && *choicedMove != move)
	{
		gCurrentMove = *choicedMove;
		gLastUsedItem = gBattleMons[gActiveBattler].item;
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedMoveChoiceItem;
		++limitations;
	}
	else if (holdEffect == ITEM_EFFECT_ASSAULT_VEST && SPLIT(move) == SPLIT_STATUS)
	{
		gLastUsedItem = gBattleMons[gActiveBattler].item;
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedMoveAssaultVest;
		++limitations;
	}
	#ifdef FLAG_SKY_BATTLE
	else if (FlagGet(FLAG_SKY_BATTLE) && CheckTableForMove(move, gSkyBattleBannedMoves))
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedSkyBattle;
		++limitations;
	}
	#endif
	else if (IsGravityActive() && CheckTableForMove(move, gGravityBannedMoves))
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedGravity;
		++limitations;
	}
	else if (CantUseSoundMoves(gActiveBattler) && CheckSoundMove(move))
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedThroatChop;
		++limitations;
	}
	else if (IsHealBlocked(gActiveBattler) && CheckHealingMove(move))
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingNotAllowedHealBlock;
		++limitations;
	}
	else if (gBattleMons[gActiveBattler].pp[gBattleBufferB[gActiveBattler][2]] == 0)
	{
		gSelectionBattleScripts[gActiveBattler] = BattleScript_SelectingMoveWithNoPP;
		++limitations;
	}

	return limitations;
}

void PlayerHandleChooseAction(void)
{
	int i;

	gNewBS->ZMoveData->toBeUsed[gActiveBattler] = FALSE;

	u16 itemId = gBattleBufferA[gActiveBattler][2] | (gBattleBufferA[gActiveBattler][3] << 8);

	if (IS_DOUBLE_BATTLE)
	{
		//Running or using balls cancels the second mon's attack
		if (!IsBagDisabled()
		&&  gBattleBufferA[gActiveBattler][1] == ACTION_USE_ITEM //If mon 1 used a ball, then
		&&  GetPocketByItemId(itemId) == POCKET_POKEBALLS)		 //mon 2 doesn't get to do anything.
		{
			gNewBS->NoMoreMovingThisTurn |= gBitTable[gActiveBattler];
			EmitTwoReturnValues(1, ACTION_USE_MOVE, 0);
			PlayerBufferExecCompleted();
			return;
		}
		else if (GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT) == gActiveBattler
		&& gBattleBufferA[gActiveBattler][1] == ACTION_RUN)
		{
			gNewBS->NoMoreMovingThisTurn |= gBitTable[gActiveBattler];
			EmitTwoReturnValues(1, ACTION_USE_MOVE, 0);
			PlayerBufferExecCompleted();
			return;
		}
	}

	gBattleBankFunc[gActiveBattler] = (u32) HandleChooseActionAfterDma3;

	if ((IS_DOUBLE_BATTLE)
	&& GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT
	&& !(gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)])
	&& !(gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER))
	&& gBattleBufferA[gActiveBattler][1] != ACTION_USE_ITEM) //Mon 1 didn't use item
	#ifndef UNBOUND
		BattlePutTextOnWindow(gText_BattleMenu2, 2);
	else
		BattlePutTextOnWindow(gText_BattleMenu, 2);
	#else
		BattlePutTextOnWindow(gText_UnboundBattleMenu2, 2);
	else
		BattlePutTextOnWindow(gText_UnboundBattleMenu, 2);
	#endif

	for (i = 0; i < 4; i++)
		ActionSelectionDestroyCursorAt(i);

	ActionSelectionCreateCursorAt(gActionSelectionCursor[gActiveBattler], 0);
	BattleStringExpandPlaceholdersToDisplayedString(gText_WhatWillPkmnDo);
	BattlePutTextOnWindow(gDisplayedStringBattle, 1);
}

void HandleInputChooseAction(void)
{
	DoBounceEffect(gActiveBattler, BOUNCE_HEALTHBOX, 7, 1);
	DoBounceEffect(gActiveBattler, BOUNCE_MON, 7, 1);

	if (gMain.newKeys & A_BUTTON)
	{
		PlaySE(SE_SELECT);

		switch (gActionSelectionCursor[gActiveBattler]) {
			case 0:
				EmitTwoReturnValues(1, ACTION_USE_MOVE, 0);
				break;
			case 1:
				gNewBS->MegaData->chosen[gActiveBattler] = FALSE;
				gNewBS->UltraData->chosen[gActiveBattler] = FALSE;
				EmitTwoReturnValues(1, ACTION_USE_ITEM, 0);
				break;
			case 2:
				gNewBS->MegaData->chosen[gActiveBattler] = FALSE;
				gNewBS->UltraData->chosen[gActiveBattler] = FALSE;
				EmitTwoReturnValues(1, ACTION_SWITCH, 0);
				break;
			case 3:
				gNewBS->MegaData->chosen[gActiveBattler] = FALSE;
				gNewBS->UltraData->chosen[gActiveBattler] = FALSE;

				if ((IS_DOUBLE_BATTLE)
				&& GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT
				&& !(gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)])
				&& !(gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER))
				&& gBattleBufferA[gActiveBattler][1] != ACTION_USE_ITEM) //Mon 1 didn't use item
					goto CANCEL_PARTNER;

				EmitTwoReturnValues(1, ACTION_RUN, 0);
				break;
		}
		PlayerBufferExecCompleted();
	}
	else if (gMain.newKeys & DPAD_LEFT)
	{
		if (gActionSelectionCursor[gActiveBattler] & 1) // if is B_ACTION_USE_ITEM or B_ACTION_RUN
		{
			PlaySE(SE_SELECT);
			ActionSelectionDestroyCursorAt(gActionSelectionCursor[gActiveBattler]);
			gActionSelectionCursor[gActiveBattler] ^= 1;
			ActionSelectionCreateCursorAt(gActionSelectionCursor[gActiveBattler], 0);
		}
	}
	else if (gMain.newKeys & DPAD_RIGHT)
	{
		if (!(gActionSelectionCursor[gActiveBattler] & 1)) // if is B_ACTION_USE_MOVE or B_ACTION_SWITCH
		{
			PlaySE(SE_SELECT);
			ActionSelectionDestroyCursorAt(gActionSelectionCursor[gActiveBattler]);
			gActionSelectionCursor[gActiveBattler] ^= 1;
			ActionSelectionCreateCursorAt(gActionSelectionCursor[gActiveBattler], 0);
		}
	}
	else if (gMain.newKeys & DPAD_UP)
	{
		if (gActionSelectionCursor[gActiveBattler] & 2) // if is B_ACTION_SWITCH or B_ACTION_RUN
		{
			PlaySE(SE_SELECT);
			ActionSelectionDestroyCursorAt(gActionSelectionCursor[gActiveBattler]);
			gActionSelectionCursor[gActiveBattler] ^= 2;
			ActionSelectionCreateCursorAt(gActionSelectionCursor[gActiveBattler], 0);
		}
	}
	else if (gMain.newKeys & DPAD_DOWN)
	{
		if (!(gActionSelectionCursor[gActiveBattler] & 2)) // if is B_ACTION_USE_MOVE or B_ACTION_USE_ITEM
		{
			PlaySE(SE_SELECT);
			ActionSelectionDestroyCursorAt(gActionSelectionCursor[gActiveBattler]);
			gActionSelectionCursor[gActiveBattler] ^= 2;
			ActionSelectionCreateCursorAt(gActionSelectionCursor[gActiveBattler], 0);
		}
	}
	else if (gMain.newKeys & B_BUTTON)
	{
		if ((IS_DOUBLE_BATTLE)
		 && GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT
		 && !(gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)])
		 && !(gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER)))
		{
		CANCEL_PARTNER:
			if (gBattleBufferA[gActiveBattler][1] == ACTION_USE_ITEM)
				return;

			PlaySE(SE_SELECT);
			EmitTwoReturnValues(1, ACTION_CANCEL_PARTNER, 0);
			PlayerBufferExecCompleted();
		}
	}
	else if (gMain.newKeys & START_BUTTON)
	{
		SwapHpBarsWithHpText();
	}
}

bool8 CheckCantMoveThisTurn(void)
{
	if (gNewBS->NoMoreMovingThisTurn & gBitTable[gActiveBattler])
	{
		EmitMoveChosen(1, 0, gMultiUsePlayerCursor, FALSE, FALSE, FALSE);
		return TRUE;
	}
	return FALSE;
}

bool8 IsBagDisabled(void)
{
	return FlagGet(FLAG_DISABLE_BAG) || (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_FRONTIER));
}

static void TryLoadTypeIcons(void)
{
	#ifndef HEALTHBAR_TYPE_ICONS
	if ((gBattleTypeFlags & BATTLE_TYPE_CAMOMONS)
	&&
	#else
	if (
	#endif
	!(gBattleTypeFlags & BATTLE_TYPE_LINK)
	&& IndexOfSpritePaletteTag(TYPE_ICON_TAG) == 0xFF)
	{
		LoadSpritePalette(&sTypeIconPalTemplate);
		LoadSpritePalette(&sTypeIconPalTemplate2);
		for (u8 position = 0; position < gBattlersCount; ++position)
		{
			if (!BATTLER_ALIVE(GetBattlerAtPosition(position)))
				continue;

			for (u8 typeNum = 0; typeNum < 2; ++typeNum) //Load each type
			{
				u8 spriteId;
				s16 x = sTypeIconPositions[position][IS_SINGLE_BATTLE].x;
				s16 y = sTypeIconPositions[position][IS_SINGLE_BATTLE].y + (11 * typeNum); //2nd type is 13px below

				u8* type1Ptr;
				if (gStatuses3[GetBattlerAtPosition(position)] & STATUS3_ILLUSION && !(gBattleTypeFlags & BATTLE_TYPE_CAMOMONS))
					type1Ptr = &gBaseStats[GetIllusionPartyData(GetBattlerAtPosition(position))->species].type1;
				else
					type1Ptr = &gBattleMons[GetBattlerAtPosition(position)].type1;

				u8 type = *(type1Ptr + typeNum);

				switch (type) { //Certain types have a different palette
					case TYPE_FLYING:
					case TYPE_POISON:
					case TYPE_GROUND:
					case TYPE_DRAGON:
					case TYPE_MYSTERY:
					case TYPE_ROOSTLESS:
					case TYPE_BLANK:
						spriteId = CreateSpriteAtEnd(&sTypeIconSpriteTemplate2, x, y, 0xFF);
						break;
					default:
						spriteId = CreateSpriteAtEnd(&sTypeIconSpriteTemplate, x, y, 0xFF);
				}

				if (spriteId != MAX_SPRITES)
				{
					struct Sprite* sprite = &gSprites[spriteId];
					sprite->data[0] = position;
					sprite->data[1] = gActiveBattler;
					sprite->data[3] = y; //Save original y-value for bouncing

					if (IS_SINGLE_BATTLE)
					{
						if (SIDE(GetBattlerAtPosition(position)) == B_SIDE_PLAYER)
							SetSpriteOamFlipBits(sprite, TRUE, FALSE); //Flip horizontally
					}
					else //Double Battle
					{
						if (SIDE(GetBattlerAtPosition(position)) == B_SIDE_OPPONENT)
							SetSpriteOamFlipBits(sprite, TRUE, FALSE); //Flip horizontally
					}

					RequestSpriteFrameImageCopy(type, sprite->oam.tileNum, sprite->images);
				}
			}
		}
	}
}

static void SpriteCB_CamomonsTypeIcon(struct Sprite* sprite)
{
	u8 position = sprite->data[0];
	u8 bank = sprite->data[1];

	if (sprite->data[2] == 10)
	{
		FreeSpritePaletteByTag(TYPE_ICON_TAG);
		DestroySprite(sprite);
		return;
	}

	//Type icons should prepare to destroy themselves if the Player is not choosing an action
	if (gBattleBankFunc[bank] != (0x08032C90 | 1) //PlayerHandleChooseMove
	&&  gBattleBankFunc[bank] != (0x0802EA10 | 1) //HandleInputChooseMove
	&&  gBattleBankFunc[bank] != (0x08032C4C | 1) //HandleChooseMoveAfterDma3
	&&  gBattleBankFunc[bank] != (u32) HandleInputChooseMove
	&&  gBattleBankFunc[bank] != (u32) HandleInputChooseTarget
	&&  gBattleBankFunc[bank] != (u32) HandleMoveSwitching
	&&  gBattleBankFunc[bank] != (u32) HandleInputChooseMove)
	{
		if (IS_SINGLE_BATTLE)
		{
			switch (position) {
				case B_POSITION_PLAYER_LEFT:
					sprite->pos1.x -= 1;
					break;
				case B_POSITION_OPPONENT_LEFT:
					sprite->pos1.x += 1;
					break;
			}
		}
		else //Double Battle
		{
			switch (position) {
				case B_POSITION_PLAYER_LEFT:
				case B_POSITION_PLAYER_RIGHT:
					sprite->pos1.x += 1;
					break;
				case B_POSITION_OPPONENT_LEFT:
				case B_POSITION_OPPONENT_RIGHT:
					sprite->pos1.x -= 1;
					break;
			}
		}

		++sprite->data[2];
		return;
	}

	if (IS_SINGLE_BATTLE)
	{
		switch (position) {
			case B_POSITION_PLAYER_LEFT:
				if (sprite->pos1.x < sTypeIconPositions[position][TRUE].x + 10)
					sprite->pos1.x += 1;
				break;
			case B_POSITION_OPPONENT_LEFT:
				if (sprite->pos1.x > sTypeIconPositions[position][TRUE].x - 10)
					sprite->pos1.x -= 1;
				break;
		}
	}
	else //Double Battle
	{
		switch (position) {
			case B_POSITION_PLAYER_LEFT:
			case B_POSITION_PLAYER_RIGHT:
				if (sprite->pos1.x > sTypeIconPositions[position][FALSE].x - 10)
					sprite->pos1.x -= 1;
				break;
			case B_POSITION_OPPONENT_LEFT:
			case B_POSITION_OPPONENT_RIGHT:
				if (sprite->pos1.x < sTypeIconPositions[position][FALSE].x + 10)
					sprite->pos1.x += 1;
				break;
		}
	}

	//Deal with bouncing player healthbox
	s16 originalY = sprite->data[3];
	struct Sprite* healthbox = &gSprites[gHealthboxIDs[GetBattlerAtPosition(position)]];
	sprite->pos1.y = originalY + healthbox->pos2.y;
}
