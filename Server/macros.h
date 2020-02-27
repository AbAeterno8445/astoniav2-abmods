/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

/* *** ITEMS *** */

// Use flag check on a STRUCTURE ELEMENT (works for characters too)
// DB: i dont know how it might fail... but i've added some parents anyway. here and in other places.
// DB: looked fairly safe before, so no need to worry
#define IS_USED(x)              ((x).used != USE_EMPTY)

// Sanity checks on item numbers
#define IS_SANEITEM(in)         ((in) > 0 && (in) < MAXITEM)
#define IS_USEDITEM(in)         (IS_USED(it[(in)]))
#define IS_SANEUSEDITEM(in)     (IS_SANEITEM(in) && IS_USEDITEM(in))



/* *** TEMPLATES *** */

// Sanity checks on item templates
#define IS_SANEITEMPLATE(tn)    ((tn) > 0 && (tn) < MAXTITEM)
#define IS_SANECTEMPLATE(tn)    ((tn) > 0 && (tn) < MAXTCHARS)



/* *** CHARACTERS *** */

// Sanity checks on character numbers
#define IS_SANECHAR(cn)         ((cn) > 0 && (cn) < MAXCHARS)
#define IS_LIVINGCHAR(cn)       (IS_SANECHAR(cn) && ch[(cn)].used != USE_EMPTY)
#define IS_ACTIVECHAR(cn)       (IS_SANECHAR(cn) && ch[(cn)].used == USE_ACTIVE)
#define IS_USEDCHAR(cn)         (IS_USED(ch[(cn)]))
#define IS_SANEUSEDCHAR(cn)     (IS_SANECHAR(cn) && IS_USEDCHAR(cn))

// flag checks
#define IS_PLAYER(cn)           ((ch[(cn)].flags & CF_PLAYER) != 0)
#define IS_STAFF(cn)            ((ch[(cn)].flags & CF_STAFF) != 0)
#define IS_GOD(cn)              ((ch[(cn)].flags & CF_GOD) != 0)
#define IS_USURP(cn)            ((ch[(cn)].flags & CF_USURP) != 0)
#define IS_IMP(cn)              ((ch[(cn)].flags & CF_IMP) != 0)
#define IS_QM(cn)               ((ch[(cn)].flags & (CF_IMP|CF_USURP)) != 0)
#define IS_IGNORING_SPELLS(cn)  ((ch[(cn)].flags & CF_SPELLIGNORE) != 0)
#define IS_CCP(cn)              ((ch[(cn)].flags & CF_CCP) != 0)
#define IS_BUILDING(cn)         ((ch[(cn)].flags & CF_BUILDMODE) != 0)

// special character group checks
#define IS_COMPANION(cn)        (IS_SANECHAR(cn) && (ch[(cn)].temp == CT_COMPANION))

// Visibility, etc.
#define IS_INVISIBLE(cn)        ((ch[(cn)].flags & CF_INVISIBLE) != 0)
#define IS_PURPLE(cn)           ((ch[(cn)].kindred & KIN_PURPLE) != 0)
#define IS_FEMALE(cn)           ((ch[cn].kindred & KIN_FEMALE) != 0)
#define HE_SHE(cn)              (IS_FEMALE(cn) ? "she" : "he")
#define HE_SHE_CAPITAL(cn)      (IS_FEMALE(cn) ? "She" : "He")
#define HIS_HER(cn)             (IS_FEMALE(cn) ? "her" : "his")
#define HIM_HER(cn)             (IS_FEMALE(cn) ? "her" : "him")

// Ditto, with sanity check
#define IS_SANEPLAYER(cn)       (IS_SANECHAR(cn) && IS_PLAYER(cn))
#define IS_SANESTAFF(cn)        (IS_SANECHAR(cn) && IS_STAFF(cn))
#define IS_SANEGOD(cn)          (IS_SANECHAR(cn) && IS_GOD(cn))
#define IS_SANEUSURP(cn)        (IS_SANECHAR(cn) && IS_USURP(cn))
// IS_SANENPC is derived. No IS_NPC because of... logic.
#define IS_SANENPC(cn)          (IS_SANECHAR(cn) && !IS_PLAYER(cn))
#define IS_SANECCP(cn)          (IS_SANECHAR(cn) && IS_CCP(cn))


/* *** SKILLS *** */

// Sanity check on skill number
#define SANESKILL(s)    ((s)>=0 && (s)<MAXSKILL)

/* *** MISC *** */

// Font macros (should match with client)
#define FNT_RED			0
#define FNT_YELLOW		1
#define FNT_GREEN		2
#define FNT_BLUE		3
#define FNT_OBFUSCATED	4
#define FNT_PURPLE		1960
#define FNT_TURQUOISE	1961
#define FNT_PINK		1962
#define FNT_ORANGE		1963
#define FNT_AQUA		1964
#define FNT_SILVER		1965
#define FNT_EMERALD		1966

// Chat font schemes
#define CFNT_LOCAL      (FNT_SILVER)    // Area chat (say)
#define CFNT_GLOBAL     (FNT_SILVER)    // Global chat (shout)
#define CFNT_TELLSEND   (FNT_YELLOW)    // Tell send
#define CFNT_TELLRECV   (FNT_YELLOW)    // Tell receive

// Character font schemes
#define CHFNT_GGOD      (FNT_TURQUOISE) // Greater god
#define CHFNT_GOD       (FNT_TURQUOISE) // God
#define CHFNT_IMP       (FNT_GREEN)     // Imp
#define CHFNT_STAFF     (FNT_GREEN)     // Staffer
#define CHFNT_BUILDER   (FNT_AQUA)      // Builder
#define CHFNT_INVIS     (FNT_AQUA)      // Invisible
#define CHFNT_NORMAL    (FNT_BLUE)      // Normal player
#define CHFNT_NPCGOOD   (FNT_EMERALD)   // Good npc (alignment > 0)
#define CHFNT_NPCEVIL   (FNT_RED)       // Evil npc (alignment < 0)

// Effect ID macros
#define FX_EVILMAGIC        5
#define FX_GOODMAGIC        6
#define FX_CASTMAGIC        7
#define FX_DEATHMIST        12
#define FX_FLRWARN_TR       13
#define FX_FLRWARN_SQ       14
#define FX_FLRWARN_CR1      15
#define FX_FLRWARN_CR2      16