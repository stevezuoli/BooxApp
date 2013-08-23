/**
 * Excel Format Manager data and access functions
 *
 * This is a crude linear database and linear lookup for now
 * The declarations in this file DO NOT OBEY THE PCS LINE LENGTH rule,
 * for reasons of clarity of definition
 *
 * Copyright (C) Picsel 2009
 *
 * @author Picsel Technologies Ltd
 *
 * $Id: excel-format.c,v 1.1 2009/03/02 13:18:51 maxim Exp $
 */

#include "stdio.h"
#include "assert.h"
#include "string.h"
#include "excel-format.h"

#define ARRAY_COUNT(array) ((int)(sizeof(array)/sizeof(array[0])))

/* Simplify array definition - declare one field */
#define FORMAT(category,sub1,sub2,sub3,rule) \
    { FORMAT_FLAGS(category,sub1,sub2,sub3),rule }

/* Eight macros to simplify array definition...
   The macro parameters should be sufficient to describe their use:
     declaring category-specific subformatting rules, and the format string.
   Note the use of token pasting to keep declarations short */

#define FNUM(negative,places,comma,rule) \
    ,FORMAT(PCLASS(Category_Number),negative,places,comma,rule)

#define FPERC(places,rule) \
    ,FORMAT(PCLASS(Category_Percentage),places,0,0,rule)

#define FSCI(places,rule) \
    ,FORMAT(PCLASS(Category_Scientific),places,0,0,rule)

#define FFRAC(style,rule) \
    ,FORMAT(PCLASS(Category_Fraction),style,0,0,rule)

#define FCUR(symbol,negative,places,rule) \
    ,FORMAT(PCLASS(Category_Currency),PCLASS(Currency_##symbol),negative,places,rule)

#define FACC(symbol,places,rule) \
    ,FORMAT(PCLASS(Category_Accounting),PCLASS(Currency_##symbol),places,0,rule)

#define FDATE(locale,format,rule) \
    ,FORMAT(PCLASS(Category_Date),PCLASS(Country_##locale),format,0,rule)

#define FTIME(locale,format,rule) \
    ,FORMAT(PCLASS(Category_Time),PCLASS(Country_##locale),format,0,rule)

/* A pairing of format string to flags */
static const struct {
    unsigned long   flags; /**< bytefield of category layers */
    const char    *format; /**< Excel formatting rule string */
} formatBlock[]={

/* General */
FORMAT(PCLASS(Category_General),0,0,0,"General"),

/* Text */
FORMAT(PCLASS(Category_Text),0,0,0,"@")

/* Number ; 4 negative groups * 4 decimal place settings * 2 comma options */
FNUM(0,0,0,"0")
FNUM(1,0,0,"0;[Red]0")
FNUM(2,0,0,"0_ ;\\-0\\ ")
FNUM(3,0,0,"0_ ;[Red]\\-0\\ ")

FNUM(0,1,0,"0.0")
FNUM(1,1,0,"0.0;[Red]0.0")
FNUM(2,1,0,"0.0_ ;\\-0.0\\ ")
FNUM(3,1,0,"0.0_ ;[Red]\\-0.0\\ ")

FNUM(0,2,0,"0.00")
FNUM(1,2,0,"0.00;[Red]0.00")
FNUM(2,2,0,"0.00_ ;\\-0.00\\ ")
FNUM(3,2,0,"0.00_ ;[Red]\\-0.00\\ ")

FNUM(0,3,0,"0.000")
FNUM(1,3,0,"0.000;[Red]0.000")
FNUM(2,3,0,"0.000_ ;\\-0.000\\ ")
FNUM(3,3,0,"0.000_ ;[Red]\\-0.000\\ ")

FNUM(0,4,0,"0.0000000000")
FNUM(1,4,0,"0.0000000000;[Red]0.0000000000")
FNUM(2,4,0,"0.0000000000_ ;\\-0.0000000000\\ ")
FNUM(3,4,0,"0.0000000000_ ;[Red]\\-0.0000000000\\ ")

FNUM(0,0,1,"#,##0")
FNUM(1,0,1,"#,##0;[Red]#,##0")
FNUM(2,0,1,"#,##0_ ;\\-#,##0\\ ")
FNUM(3,0,1,"#,##0_ ;[Red]\\-#,##0\\ ")

FNUM(0,1,1,"#,##0.0")
FNUM(1,1,1,"#,##0.0;[Red]#,##0.0")
FNUM(2,1,1,"#,##0.0_ ;\\-#,##0.0\\ ")
FNUM(3,1,1,"#,##0.0_ ;[Red]\\-#,##0.0\\ ")

FNUM(0,2,1,"#,##0.00")
FNUM(1,2,1,"#,##0.00;[Red]#,##0.00")
FNUM(2,2,1,"#,##0.00_ ;\\-#,##0.00\\ ")
FNUM(3,2,1,"#,##0.00_ ;[Red]\\-#,##0.00\\ ")

FNUM(0,3,1,"#,##0.000")
FNUM(1,3,1,"#,##0.000;[Red]#,##0.000")
FNUM(2,3,1,"#,##0.000_ ;\\-#,##0.000\\ ")
FNUM(3,3,1,"#,##0.000_ ;[Red]\\-#,##0.000\\ ")

FNUM(0,4,1,"#,##0.0000000000")
FNUM(1,4,1,"#,##0.0000000000;[Red]#,##0.0000000000")
FNUM(2,4,1,"#,##0.0000000000_ ;\\-#,##0.0000000000\\ ")
FNUM(3,4,1,"#,##0.0000000000_ ;[Red]\\-#,##0.0000000000\\ ")

/* Five Percentage options */
FPERC(0,"0%")
FPERC(1,"0.0%")
FPERC(2,"0.00%")
FPERC(3,"0.000%")
FPERC(4,"0.0000000000%")

/* Five Scientific options */
FSCI(0,"0.E+00")
FSCI(1,"0.0E+00")
FSCI(2,"0.00E+00")
FSCI(3,"0.000E+00")
FSCI(4,"0.0000000000E+00")

/* Nine Fraction options - the first two rules are 'odd' */
FFRAC(0,"# \?/\?")
FFRAC(1,"# \?\?/?\?")
FFRAC(2,"#\\ \?\?\?/\?\?\?")
FFRAC(3,"#\\ \?/2")
FFRAC(4,"#\\ \?/4")
FFRAC(5,"#\\ \?/8")
FFRAC(6,"#\\ \?\?/16")
FFRAC(7,"#\\ \?/10")
FFRAC(8,"#\\ \?\?/100")

/* Currency ; 53 symbols * 4 negative groups * 2 decimal place settings */
FCUR(Brazil_Real,0,0,"[$R$-416]\\ #,##0")
FCUR(Brazil_Real,1,0,"[$R$-416]\\ #,##0;[Red][$R$-416]\\ #,##0")
FCUR(Brazil_Real,2,0,"[$R$-416]\\ #,##0;\\-[$R$-416]\\ #,##0")
FCUR(Brazil_Real,3,0,"[$R$-416]\\ #,##0;[Red]\\-[$R$-416]\\ #,##0")
FCUR(Brazil_Real,0,1,"[$R$-416]\\ #,##0.00")
FCUR(Brazil_Real,1,1,"[$R$-416]\\ #,##0.00;[Red][$R$-416]\\ #,##0.00")
FCUR(Brazil_Real,2,1,"[$R$-416]\\ #,##0.00;\\-[$R$-416]\\ #,##0.00")
FCUR(Brazil_Real,3,1,"[$R$-416]\\ #,##0.00;[Red]\\-[$R$-416]\\ #,##0.00")

FCUR(Uruguay_Peso,0,0,"[$$U-380A]\\ #,##0")
FCUR(Uruguay_Peso,1,0,"[$$U-380A]\\ #,##0;[Red][$$U-380A]\\ #,##0")
FCUR(Uruguay_Peso,2,0,"[$$U-380A]\\ #,##0;[$$U-380A]\\ \\-#,##0")
FCUR(Uruguay_Peso,3,0,"[$$U-380A]\\ #,##0;[Red][$$U-380A]\\ \\-#,##0")
FCUR(Uruguay_Peso,0,1,"[$$U-380A]\\ #,##0.00")
FCUR(Uruguay_Peso,1,1,"[$$U-380A]\\ #,##0.00;[Red][$$U-380A]\\ #,##0.00")
FCUR(Uruguay_Peso,2,1,"[$$U-380A]\\ #,##0.00;[$$U-380A]\\ \\-#,##0.00")
FCUR(Uruguay_Peso,3,1,"[$$U-380A]\\ #,##0.00;[Red][$$U-380A]\\ \\-#,##0.00")

FCUR(Argentina_Peso,0,0,"[$$-2C0A]\\ #,##0")
FCUR(Argentina_Peso,1,0,"[$$-2C0A]\\ #,##0;[Red][$$-2C0A]\\ #,##0")
FCUR(Argentina_Peso,2,0,"[$$-2C0A]\\ #,##0;[$$-2C0A]\\ \\-#,##0")
FCUR(Argentina_Peso,3,0,"[$$-2C0A]\\ #,##0;[Red][$$-2C0A]\\ \\-#,##0")
FCUR(Argentina_Peso,0,1,"[$$-2C0A]\\ #,##0.00")
FCUR(Argentina_Peso,1,1,"[$$-2C0A]\\ #,##0.00;[Red][$$-2C0A]\\ #,##0.00")
FCUR(Argentina_Peso,2,1,"[$$-2C0A]\\ #,##0.00;[$$-2C0A]\\ \\-#,##0.00")
FCUR(Argentina_Peso,3,1,"[$$-2C0A]\\ #,##0.00;[Red][$$-2C0A]\\ \\-#,##0.00")

FCUR(Colombia_Peso,0,0,"[$$-240A]\\ #,##0")
FCUR(Colombia_Peso,1,0,"[$$-240A]\\ #,##0;[Red][$$-240A]\\ #,##0")
FCUR(Colombia_Peso,2,0,"[$$-240A]\\ #,##0;[$$-240A]\\ \\-#,##0")
FCUR(Colombia_Peso,3,0,"[$$-240A]\\ #,##0;[Red][$$-240A]\\ \\-#,##0")
FCUR(Colombia_Peso,0,1,"[$$-240A]\\ #,##0.00")
FCUR(Colombia_Peso,1,1,"[$$-240A]\\ #,##0.00;[Red][$$-240A]\\ #,##0.00")
FCUR(Colombia_Peso,2,1,"[$$-240A]\\ #,##0.00;[$$-240A]\\ \\-#,##0.00")
FCUR(Colombia_Peso,3,1,"[$$-240A]\\ #,##0.00;[Red][$$-240A]\\ \\-#,##0.00")

FCUR(Guatemala_Quetzal,0,0,"[$Q-100A]#,##0")
FCUR(Guatemala_Quetzal,1,0,"[$Q-100A]#,##0;[Red][$Q-100A]#,##0")
FCUR(Guatemala_Quetzal,2,0,"[$Q-100A]#,##0_ ;\\-[$Q-100A]#,##0\\ ")
FCUR(Guatemala_Quetzal,3,0,"[$Q-100A]#,##0_ ;[Red]\\-[$Q-100A]#,##0\\ ")
FCUR(Guatemala_Quetzal,0,1,"[$Q-100A]#,##0.00")
FCUR(Guatemala_Quetzal,1,1,"[$Q-100A]#,##0.00;[Red][$Q-100A]#,##0.00")
FCUR(Guatemala_Quetzal,2,1,"[$Q-100A]#,##0.00_ ;\\-[$Q-100A]#,##0.00\\ ")
FCUR(Guatemala_Quetzal,3,1,"[$Q-100A]#,##0.00_ ;[Red]\\-[$Q-100A]#,##0.00\\ ")

FCUR(Bolivia_Boliviano_Quechua,0,0,"[$$b-46B]\\ #,##0")
FCUR(Bolivia_Boliviano_Quechua,1,0,"[$$b-46B]\\ #,##0;[Red][$$b-46B]\\ #,##0")
FCUR(Bolivia_Boliviano_Quechua,2,0,"[$$b-46B]\\ #,##0;[$$b-46B]\\ \\-#,##0")
FCUR(Bolivia_Boliviano_Quechua,3,0,"[$$b-46B]\\ #,##0;[Red][$$b-46B]\\ \\-#,##0")
FCUR(Bolivia_Boliviano_Quechua,0,1,"[$$b-46B]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Quechua,1,1,"[$$b-46B]\\ #,##0.00;[Red][$$b-46B]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Quechua,2,1,"[$$b-46B]\\ #,##0.00;[$$b-46B]\\ \\-#,##0.00")
FCUR(Bolivia_Boliviano_Quechua,3,1,"[$$b-46B]\\ #,##0.00;[Red][$$b-46B]\\ \\-#,##0.00")

FCUR(Bolivia_Boliviano_Spanish,0,0,"[$$b-400A]\\ #,##0")
FCUR(Bolivia_Boliviano_Spanish,1,0,"[$$b-400A]\\ #,##0;[Red][$$b-400A]\\ #,##0")
FCUR(Bolivia_Boliviano_Spanish,2,0,"[$$b-400A]\\ #,##0;[$$b-400A]\\ \\-#,##0")
FCUR(Bolivia_Boliviano_Spanish,3,0,"[$$b-400A]\\ #,##0;[Red][$$b-400A]\\ \\-#,##0")
FCUR(Bolivia_Boliviano_Spanish,0,1,"[$$b-400A]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Spanish,1,1,"[$$b-400A]\\ #,##0.00;[Red][$$b-400A]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Spanish,2,1,"[$$b-400A]\\ #,##0.00;[$$b-400A]\\ \\-#,##0.00")
FCUR(Bolivia_Boliviano_Spanish,3,1,"[$$b-400A]\\ #,##0.00;[Red][$$b-400A]\\ \\-#,##0.00")

FCUR(Chile_Peso,0,0,"[$$-340A]\\ #,##0")
FCUR(Chile_Peso,1,0,"[$$-340A]\\ #,##0;[Red][$$-340A]\\ #,##0")
FCUR(Chile_Peso,2,0,"[$$-340A]\\ #,##0;\\-[$$-340A]\\ #,##0")
FCUR(Chile_Peso,3,0,"[$$-340A]\\ #,##0;[Red]\\-[$$-340A]\\ #,##0")
FCUR(Chile_Peso,0,1,"[$$-340A]\\ #,##0.00")
FCUR(Chile_Peso,1,1,"[$$-340A]\\ #,##0.00;[Red][$$-340A]\\ #,##0.00")
FCUR(Chile_Peso,2,1,"[$$-340A]\\ #,##0.00;\\-[$$-340A]\\ #,##0.00")
FCUR(Chile_Peso,3,1,"[$$-340A]\\ #,##0.00;[Red]\\-[$$-340A]\\ #,##0.00")

FCUR(DominicanRepublic_Peso,0,0,"[$RD$-1C0A]#,##0")
FCUR(DominicanRepublic_Peso,1,0,"[$RD$-1C0A]#,##0;[Red][$RD$-1C0A]#,##0")
FCUR(DominicanRepublic_Peso,2,0,"[$RD$-1C0A]#,##0_ ;\\-[$RD$-1C0A]#,##0\\ ")
FCUR(DominicanRepublic_Peso,3,0,"[$RD$-1C0A]#,##0_ ;[Red]\\-[$RD$-1C0A]#,##0\\ ")
FCUR(DominicanRepublic_Peso,0,1,"[$RD$-1C0A]#,##0.00")
FCUR(DominicanRepublic_Peso,1,1,"[$RD$-1C0A]#,##0.00;[Red][$RD$-1C0A]#,##0.00")
FCUR(DominicanRepublic_Peso,2,1,"[$RD$-1C0A]#,##0.00_ ;\\-[$RD$-1C0A]#,##0.00\\ ")
FCUR(DominicanRepublic_Peso,3,1,"[$RD$-1C0A]#,##0.00_ ;[Red]\\-[$RD$-1C0A]#,##0.00\\ ")

FCUR(Ecuador_Sucre_Quechua,0,0,"[$$-86B]\\ #,##0")
FCUR(Ecuador_Sucre_Quechua,1,0,"[$$-86B]\\ #,##0;[Red][$$-86B]\\ #,##0")
FCUR(Ecuador_Sucre_Quechua,2,0,"[$$-86B]\\ #,##0;[$$-86B]\\ \\-#,##0")
FCUR(Ecuador_Sucre_Quechua,3,0,"[$$-86B]\\ #,##0;[Red][$$-86B]\\ \\-#,##0")
FCUR(Ecuador_Sucre_Quechua,0,1,"[$$-86B]\\ #,##0.00")
FCUR(Ecuador_Sucre_Quechua,1,1,"[$$-86B]\\ #,##0.00;[Red][$$-86B]\\ #,##0.00")
FCUR(Ecuador_Sucre_Quechua,2,1,"[$$-86B]\\ #,##0.00;[$$-86B]\\ \\-#,##0.00")
FCUR(Ecuador_Sucre_Quechua,3,1,"[$$-86B]\\ #,##0.00;[Red][$$-86B]\\ \\-#,##0.00")

FCUR(Ecuador_Sucre_Spanish,0,0,"[$$-300A]\\ #,##0")
FCUR(Ecuador_Sucre_Spanish,1,0,"[$$-300A]\\ #,##0;[Red][$$-300A]\\ #,##0")
FCUR(Ecuador_Sucre_Spanish,2,0,"[$$-300A]\\ #,##0;[$$-300A]\\ \\-#,##0")
FCUR(Ecuador_Sucre_Spanish,3,0,"[$$-300A]\\ #,##0;[Red][$$-300A]\\ \\-#,##0")
FCUR(Ecuador_Sucre_Spanish,0,1,"[$$-300A]\\ #,##0.00")
FCUR(Ecuador_Sucre_Spanish,1,1,"[$$-300A]\\ #,##0.00;[Red][$$-300A]\\ #,##0.00")
FCUR(Ecuador_Sucre_Spanish,2,1,"[$$-300A]\\ #,##0.00;[$$-300A]\\ \\-#,##0.00")
FCUR(Ecuador_Sucre_Spanish,3,1,"[$$-300A]\\ #,##0.00;[Red][$$-300A]\\ \\-#,##0.00")

FCUR(Mexico_Peso,0,0,"[$$-80A]#,##0")
FCUR(Mexico_Peso,1,0,"[$$-80A]#,##0;[Red][$$-80A]#,##0")
FCUR(Mexico_Peso,2,0,"[$$-80A]#,##0;\\-[$$-80A]#,##0")
FCUR(Mexico_Peso,3,0,"[$$-80A]#,##0;[Red]\\-[$$-80A]#,##0")
FCUR(Mexico_Peso,0,1,"[$$-80A]#,##0.00")
FCUR(Mexico_Peso,1,1,"[$$-80A]#,##0.00;[Red][$$-80A]#,##0.00")
FCUR(Mexico_Peso,2,1,"[$$-80A]#,##0.00;\\-[$$-80A]#,##0.00")
FCUR(Mexico_Peso,3,1,"[$$-80A]#,##0.00;[Red]\\-[$$-80A]#,##0.00")

FCUR(Peru_NuevoSol_Quechua,0,0,"[$S/.-C6B]\\ #,##0")
FCUR(Peru_NuevoSol_Quechua,1,0,"[$S/.-C6B]\\ #,##0;[Red][$S/.-C6B]\\ #,##0")
FCUR(Peru_NuevoSol_Quechua,2,0,"[$S/.-C6B]\\ #,##0_ ;\\-[$S/.-C6B]\\ #,##0\\ ")
FCUR(Peru_NuevoSol_Quechua,3,0,"[$S/.-C6B]\\ #,##0_ ;[Red]\\-[$S/.-C6B]\\ #,##0\\ ")
FCUR(Peru_NuevoSol_Quechua,0,1,"[$S/.-C6B]\\ #,##0.00")
FCUR(Peru_NuevoSol_Quechua,1,1,"[$S/.-C6B]\\ #,##0.00;[Red][$S/.-C6B]\\ #,##0.00")
FCUR(Peru_NuevoSol_Quechua,2,1,"[$S/.-C6B]\\ #,##0.00_ ;\\-[$S/.-C6B]\\ #,##0.00\\ ")
FCUR(Peru_NuevoSol_Quechua,3,1,"[$S/.-C6B]\\ #,##0.00_ ;[Red]\\-[$S/.-C6B]\\ #,##0.00\\ ")

FCUR(Peru_NuevoSol_Spanish,0,0,"[$S/.-280A]\\ #,##0")
FCUR(Peru_NuevoSol_Spanish,1,0,"[$S/.-280A]\\ #,##0;[Red][$S/.-280A]\\ #,##0")
FCUR(Peru_NuevoSol_Spanish,2,0,"[$S/.-280A]\\ #,##0_ ;\\-[$S/.-280A]\\ #,##0\\ ")
FCUR(Peru_NuevoSol_Spanish,3,0,"[$S/.-280A]\\ #,##0_ ;[Red]\\-[$S/.-280A]\\ #,##0\\ ")
FCUR(Peru_NuevoSol_Spanish,0,1,"[$S/.-280A]\\ #,##0.00")
FCUR(Peru_NuevoSol_Spanish,1,1,"[$S/.-280A]\\ #,##0.00;[Red][$S/.-280A]\\ #,##0.00")
FCUR(Peru_NuevoSol_Spanish,2,1,"[$S/.-280A]\\ #,##0.00_ ;\\-[$S/.-280A]\\ #,##0.00\\ ")
FCUR(Peru_NuevoSol_Spanish,3,1,"[$S/.-280A]\\ #,##0.00_ ;[Red]\\-[$S/.-280A]\\ #,##0.00\\ ")

FCUR(Paraguay_Guarani,0,0,"[$Gs-3C0A]\\ #,##0")
FCUR(Paraguay_Guarani,1,0,"[$Gs-3C0A]\\ #,##0;[Red][$Gs-3C0A]\\ #,##0")
FCUR(Paraguay_Guarani,2,0,"[$Gs-3C0A]\\ #,##0;[$Gs-3C0A]\\ \\-#,##0")
FCUR(Paraguay_Guarani,3,0,"[$Gs-3C0A]\\ #,##0;[Red][$Gs-3C0A]\\ \\-#,##0")
FCUR(Paraguay_Guarani,0,1,"[$Gs-3C0A]\\ #,##0.00")
FCUR(Paraguay_Guarani,1,1,"[$Gs-3C0A]\\ #,##0.00;[Red][$Gs-3C0A]\\ #,##0.00")
FCUR(Paraguay_Guarani,2,1,"[$Gs-3C0A]\\ #,##0.00;[$Gs-3C0A]\\ \\-#,##0.00")
FCUR(Paraguay_Guarani,3,1,"[$Gs-3C0A]\\ #,##0.00;[Red][$Gs-3C0A]\\ \\-#,##0.00")

FCUR(ElSalvador_Colon,0,0,"[$$-440A]#,##0")
FCUR(ElSalvador_Colon,1,0,"[$$-440A]#,##0;[Red][$$-440A]#,##0")
FCUR(ElSalvador_Colon,2,0,"[$$-440A]#,##0_ ;\\-[$$-440A]#,##0\\ ")
FCUR(ElSalvador_Colon,3,0,"[$$-440A]#,##0_ ;[Red]\\-[$$-440A]#,##0\\ ")
FCUR(ElSalvador_Colon,0,1,"[$$-440A]#,##0.00")
FCUR(ElSalvador_Colon,1,1,"[$$-440A]#,##0.00;[Red][$$-440A]#,##0.00")
FCUR(ElSalvador_Colon,2,1,"[$$-440A]#,##0.00_ ;\\-[$$-440A]#,##0.00\\ ")
FCUR(ElSalvador_Colon,3,1,"[$$-440A]#,##0.00_ ;[Red]\\-[$$-440A]#,##0.00\\ ")

FCUR(TrinidadTobago_Dollar,0,0,"[$TT$-2C09]#,##0")
FCUR(TrinidadTobago_Dollar,1,0,"[$TT$-2C09]#,##0;[Red][$TT$-2C09]#,##0")
FCUR(TrinidadTobago_Dollar,2,0,"[$TT$-2C09]#,##0_ ;\\-[$TT$-2C09]#,##0\\ ")
FCUR(TrinidadTobago_Dollar,3,0,"[$TT$-2C09]#,##0_ ;[Red]\\-[$TT$-2C09]#,##0\\ ")
FCUR(TrinidadTobago_Dollar,0,1,"[$TT$-2C09]#,##0.00")
FCUR(TrinidadTobago_Dollar,1,1,"[$TT$-2C09]#,##0.00;[Red][$TT$-2C09]#,##0.00")
FCUR(TrinidadTobago_Dollar,2,1,"[$TT$-2C09]#,##0.00_ ;\\-[$TT$-2C09]#,##0.00\\ ")
FCUR(TrinidadTobago_Dollar,3,1,"[$TT$-2C09]#,##0.00_ ;[Red]\\-[$TT$-2C09]#,##0.00\\ ")

FCUR(Honduras_Lempira,0,0,"[$L.-480A]\\ #,##0")
FCUR(Honduras_Lempira,1,0,"[$L.-480A]\\ #,##0;[Red][$L.-480A]\\ #,##0")
FCUR(Honduras_Lempira,2,0,"[$L.-480A]\\ #,##0_ ;\\-[$L.-480A]\\ #,##0\\ ")
FCUR(Honduras_Lempira,3,0,"[$L.-480A]\\ #,##0_ ;[Red]\\-[$L.-480A]\\ #,##0\\ ")
FCUR(Honduras_Lempira,0,1,"[$L.-480A]\\ #,##0.00")
FCUR(Honduras_Lempira,1,1,"[$L.-480A]\\ #,##0.00;[Red][$L.-480A]\\ #,##0.00")
FCUR(Honduras_Lempira,2,1,"[$L.-480A]\\ #,##0.00_ ;\\-[$L.-480A]\\ #,##0.00\\ ")
FCUR(Honduras_Lempira,3,1,"[$L.-480A]\\ #,##0.00_ ;[Red]\\-[$L.-480A]\\ #,##0.00\\ ")

FCUR(Nicaragua_Cordoba,0,0,"[$C$-4C0A]\\ #,##0")
FCUR(Nicaragua_Cordoba,1,0,"[$C$-4C0A]\\ #,##0;[Red][$C$-4C0A]\\ #,##0")
FCUR(Nicaragua_Cordoba,2,0,"[$C$-4C0A]\\ #,##0;[$C$-4C0A]\\ \\-#,##0")
FCUR(Nicaragua_Cordoba,3,0,"[$C$-4C0A]\\ #,##0;[Red][$C$-4C0A]\\ \\-#,##0")
FCUR(Nicaragua_Cordoba,0,1,"[$C$-4C0A]\\ #,##0.00")
FCUR(Nicaragua_Cordoba,1,1,"[$C$-4C0A]\\ #,##0.00;[Red][$C$-4C0A]\\ #,##0.00")
FCUR(Nicaragua_Cordoba,2,1,"[$C$-4C0A]\\ #,##0.00;[$C$-4C0A]\\ \\-#,##0.00")
FCUR(Nicaragua_Cordoba,3,1,"[$C$-4C0A]\\ #,##0.00;[Red][$C$-4C0A]\\ \\-#,##0.00")

FCUR(Venezuela_Bolivar,0,0,"[$Bs-200A]\\ #,##0")
FCUR(Venezuela_Bolivar,1,0,"[$Bs-200A]\\ #,##0;[Red][$Bs-200A]\\ #,##0")
FCUR(Venezuela_Bolivar,2,0,"[$Bs-200A]\\ #,##0_ ;\\-[$Bs-200A]\\ #,##0\\ ")
FCUR(Venezuela_Bolivar,3,0,"[$Bs-200A]\\ #,##0_ ;[Red]\\-[$Bs-200A]\\ #,##0\\ ")
FCUR(Venezuela_Bolivar,0,1,"[$Bs-200A]\\ #,##0.00")
FCUR(Venezuela_Bolivar,1,1,"[$Bs-200A]\\ #,##0.00;[Red][$Bs-200A]\\ #,##0.00")
FCUR(Venezuela_Bolivar,2,1,"[$Bs-200A]\\ #,##0.00_ ;\\-[$Bs-200A]\\ #,##0.00\\ ")
FCUR(Venezuela_Bolivar,3,1,"[$Bs-200A]\\ #,##0.00_ ;[Red]\\-[$Bs-200A]\\ #,##0.00\\ ")

FCUR(Panama_Balboa,0,0,"[$B/.-180A]\\ #,##0")
FCUR(Panama_Balboa,1,0,"[$B/.-180A]\\ #,##0;[Red][$B/.-180A]\\ #,##0")
FCUR(Panama_Balboa,2,0,"[$B/.-180A]\\ #,##0;[$B/.-180A]\\ \\-#,##0")
FCUR(Panama_Balboa,3,0,"[$B/.-180A]\\ #,##0;[Red][$B/.-180A]\\ \\-#,##0")
FCUR(Panama_Balboa,0,1,"[$B/.-180A]\\ #,##0.00")
FCUR(Panama_Balboa,1,1,"[$B/.-180A]\\ #,##0.00;[Red][$B/.-180A]\\ #,##0.00")
FCUR(Panama_Balboa,2,1,"[$B/.-180A]\\ #,##0.00;[$B/.-180A]\\ \\-#,##0.00")
FCUR(Panama_Balboa,3,1,"[$B/.-180A]\\ #,##0.00;[Red][$B/.-180A]\\ \\-#,##0.00")

FCUR(CostaRica_Colon,0,0,"[$\xe2\x82\xa1-140A]#,##0")
FCUR(CostaRica_Colon,1,0,"[$\xe2\x82\xa1-140A]#,##0;[Red][$\xe2\x82\xa1-140A]#,##0")
FCUR(CostaRica_Colon,2,0,"[$\xe2\x82\xa1-140A]#,##0_ ;\\-[$\xe2\x82\xa1-140A]#,##0\\ ")
FCUR(CostaRica_Colon,3,0,"[$\xe2\x82\xa1-140A]#,##0_ ;[Red]\\-[$\xe2\x82\xa1-140A]#,##0\\ ")
FCUR(CostaRica_Colon,0,1,"[$\xe2\x82\xa1-140A]#,##0.00")
FCUR(CostaRica_Colon,1,1,"[$\xe2\x82\xa1-140A]#,##0.00;[Red][$\xe2\x82\xa1-140A]#,##0.00")
FCUR(CostaRica_Colon,2,1,"[$\xe2\x82\xa1-140A]#,##0.00_ ;\\-[$\xe2\x82\xa1-140A]#,##0.00\\ ")
FCUR(CostaRica_Colon,3,1,"[$\xe2\x82\xa1-140A]#,##0.00_ ;[Red]\\-[$\xe2\x82\xa1-140A]#,##0.00\\ ")

FCUR(Jamaica_Dollar,0,0,"[$J$-2009]#,##0")
FCUR(Jamaica_Dollar,1,0,"[$J$-2009]#,##0;[Red][$J$-2009]#,##0")
FCUR(Jamaica_Dollar,2,0,"[$J$-2009]#,##0;\\-[$J$-2009]#,##0")
FCUR(Jamaica_Dollar,3,0,"[$J$-2009]#,##0;[Red]\\-[$J$-2009]#,##0")
FCUR(Jamaica_Dollar,0,1,"[$J$-2009]#,##0.00")
FCUR(Jamaica_Dollar,1,1,"[$J$-2009]#,##0.00;[Red][$J$-2009]#,##0.00")
FCUR(Jamaica_Dollar,2,1,"[$J$-2009]#,##0.00;\\-[$J$-2009]#,##0.00")
FCUR(Jamaica_Dollar,3,1,"[$J$-2009]#,##0.00;[Red]\\-[$J$-2009]#,##0.00")

FCUR(FrenchCanada_Dollar,0,0,"#,##0\\ [$$-C0C]")
FCUR(FrenchCanada_Dollar,1,0,"#,##0\\ [$$-C0C];[Red]#,##0\\ [$$-C0C]")
FCUR(FrenchCanada_Dollar,2,0,"#,##0\\ [$$-C0C]_ ;\\-#,##0\\ [$$-C0C]\\ ")
FCUR(FrenchCanada_Dollar,3,0,"#,##0\\ [$$-C0C]_ ;[Red]\\-#,##0\\ [$$-C0C]\\ ")
FCUR(FrenchCanada_Dollar,0,1,"#,##0.00\\ [$$-C0C]")
FCUR(FrenchCanada_Dollar,1,1,"#,##0.00\\ [$$-C0C];[Red]#,##0.00\\ [$$-C0C]")
FCUR(FrenchCanada_Dollar,2,1,"#,##0.00\\ [$$-C0C]_ ;\\-#,##0.00\\ [$$-C0C]\\ ")
FCUR(FrenchCanada_Dollar,3,1,"#,##0.00\\ [$$-C0C]_ ;[Red]\\-#,##0.00\\ [$$-C0C]\\ ")

FCUR(Sterling,0,0,"\"\xc2\xa3\"#,##0")
FCUR(Sterling,1,0,"\"\xc2\xa3\"#,##0;[Red]\"\xc2\xa3\"#,##0")
FCUR(Sterling,2,0,"\"\xc2\xa3\"#,##0;\\-\"\xc2\xa3\"#,##0")
FCUR(Sterling,3,0,"\"\xc2\xa3\"#,##0;[Red]\\-\"\xc2\xa3\"#,##0")
FCUR(Sterling,0,1,"\"\xc2\xa3\"#,##0.00")
FCUR(Sterling,1,1,"\"\xc2\xa3\"#,##0.00;[Red]\"\xc2\xa3\"#,##0.00")
FCUR(Sterling,2,1,"\"\xc2\xa3\"#,##0.00;\\-\"\xc2\xa3\"#,##0.00")
FCUR(Sterling,3,1,"\"\xc2\xa3\"#,##0.00;[Red]\\-\"\xc2\xa3\"#,##0.00")

FCUR(UnitedKingdom_Pound,0,0,"[$\xc2\xa3-809]#,##0")
FCUR(UnitedKingdom_Pound,1,0,"[$\xc2\xa3-809]#,##0;[Red][$\xc2\xa3-809]#,##0")
FCUR(UnitedKingdom_Pound,2,0,"[$\xc2\xa3-809]#,##0;\\-[$\xc2\xa3-809]#,##0")
FCUR(UnitedKingdom_Pound,3,0,"[$\xc2\xa3-809]#,##0;[Red]\\-[$\xc2\xa3-809]#,##0")
FCUR(UnitedKingdom_Pound,0,1,"[$\xc2\xa3-809]#,##0.00")
FCUR(UnitedKingdom_Pound,1,1,"[$\xc2\xa3-809]#,##0.00;[Red][$\xc2\xa3-809]#,##0.00")
FCUR(UnitedKingdom_Pound,2,1,"[$\xc2\xa3-809]#,##0.00;\\-[$\xc2\xa3-809]#,##0.00")
FCUR(UnitedKingdom_Pound,3,1,"[$\xc2\xa3-809]#,##0.00;[Red]\\-[$\xc2\xa3-809]#,##0.00")

FCUR(UnitedStates_Dollar,0,0,"[$$-409]#,##0")
FCUR(UnitedStates_Dollar,1,0,"[$$-409]#,##0;[Red][$$-409]#,##0")
FCUR(UnitedStates_Dollar,2,0,"[$$-409]#,##0_ ;\\-[$$-409]#,##0\\ ")
FCUR(UnitedStates_Dollar,3,0,"[$$-409]#,##0_ ;[Red]\\-[$$-409]#,##0\\ ")
FCUR(UnitedStates_Dollar,0,1,"[$$-409]#,##0.00")
FCUR(UnitedStates_Dollar,1,1,"[$$-409]#,##0.00;[Red][$$-409]#,##0.00")
FCUR(UnitedStates_Dollar,2,1,"[$$-409]#,##0.00_ ;\\-[$$-409]#,##0.00\\ ")
FCUR(UnitedStates_Dollar,3,1,"[$$-409]#,##0.00_ ;[Red]\\-[$$-409]#,##0.00\\ ")

FCUR(Euro,0,0,"[$\xe2\x82\xac-2]\\ #,##0")
FCUR(Euro,1,0,"[$\xe2\x82\xac-2]\\ #,##0;[Red][$\xe2\x82\xac-2]\\ #,##0")
FCUR(Euro,2,0,"[$\xe2\x82\xac-2]\\ #,##0;\\-[$\xe2\x82\xac-2]\\ #,##0")
FCUR(Euro,3,0,"[$\xe2\x82\xac-2]\\ #,##0;[Red]\\-[$\xe2\x82\xac-2]\\ #,##0")
FCUR(Euro,0,1,"[$\xe2\x82\xac-2]\\ #,##0.00")
FCUR(Euro,1,1,"[$\xe2\x82\xac-2]\\ #,##0.00;[Red][$\xe2\x82\xac-2]\\ #,##0.00")
FCUR(Euro,2,1,"[$\xe2\x82\xac-2]\\ #,##0.00;\\-[$\xe2\x82\xac-2]\\ #,##0.00")
FCUR(Euro,3,1,"[$\xe2\x82\xac-2]\\ #,##0.00;[Red]\\-[$\xe2\x82\xac-2]\\ #,##0.00")

FCUR(Euro_Alt,0,0,"#,##0\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,1,0,"#,##0\\ [$\xe2\x82\xac-1];[Red]#,##0\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,2,0,"#,##0\\ [$\xe2\x82\xac-1];\\-#,##0\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,3,0,"#,##0\\ [$\xe2\x82\xac-1];[Red]\\-#,##0\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,0,1,"#,##0.00\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,1,1,"#,##0.00\\ [$\xe2\x82\xac-1];[Red]#,##0.00\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,2,1,"#,##0.00\\ [$\xe2\x82\xac-1];\\-#,##0.00\\ [$\xe2\x82\xac-1]")
FCUR(Euro_Alt,3,1,"#,##0.00\\ [$\xe2\x82\xac-1];[Red]\\-#,##0.00\\ [$\xe2\x82\xac-1]")

FCUR(Brazil_Real_Name,0,0,"[$BRL]\\ #,##0")
FCUR(Brazil_Real_Name,1,0,"[$BRL]\\ #,##0;[Red][$BRL]\\ #,##0")
FCUR(Brazil_Real_Name,2,0,"[$BRL]\\ #,##0;\\-[$BRL]\\ #,##0")
FCUR(Brazil_Real_Name,3,0,"[$BRL]\\ #,##0;[Red]\\-[$BRL]\\ #,##0")
FCUR(Brazil_Real_Name,0,1,"[$BRL]\\ #,##0.00")
FCUR(Brazil_Real_Name,1,1,"[$BRL]\\ #,##0.00;[Red][$BRL]\\ #,##0.00")
FCUR(Brazil_Real_Name,2,1,"[$BRL]\\ #,##0.00;\\-[$BRL]\\ #,##0.00")
FCUR(Brazil_Real_Name,3,1,"[$BRL]\\ #,##0.00;[Red]\\-[$BRL]\\ #,##0.00")

FCUR(Uruguay_Peso_Name,0,0,"[$UYU]\\ #,##0")
FCUR(Uruguay_Peso_Name,1,0,"[$UYU]\\ #,##0;[Red][$UYU]\\ #,##0")
FCUR(Uruguay_Peso_Name,2,0,"[$UYU]\\ #,##0;\\-[$UYU]\\ #,##0")
FCUR(Uruguay_Peso_Name,3,0,"[$UYU]\\ #,##0;[Red]\\-[$UYU]\\ #,##0")
FCUR(Uruguay_Peso_Name,0,1,"[$UYU]\\ #,##0.00")
FCUR(Uruguay_Peso_Name,1,1,"[$UYU]\\ #,##0.00;[Red][$UYU]\\ #,##0.00")
FCUR(Uruguay_Peso_Name,2,1,"[$UYU]\\ #,##0.00;\\-[$UYU]\\ #,##0.00")
FCUR(Uruguay_Peso_Name,3,1,"[$UYU]\\ #,##0.00;[Red]\\-[$UYU]\\ #,##0.00")

FCUR(Argentina_Peso_Name,0,0,"[$ARS]\\ #,##0")
FCUR(Argentina_Peso_Name,1,0,"[$ARS]\\ #,##0;[Red][$ARS]\\ #,##0")
FCUR(Argentina_Peso_Name,2,0,"[$ARS]\\ #,##0;\\-[$ARS]\\ #,##0")
FCUR(Argentina_Peso_Name,3,0,"[$ARS]\\ #,##0;[Red]\\-[$ARS]\\ #,##0")
FCUR(Argentina_Peso_Name,0,1,"[$ARS]\\ #,##0.00")
FCUR(Argentina_Peso_Name,1,1,"[$ARS]\\ #,##0.00;[Red][$ARS]\\ #,##0.00")
FCUR(Argentina_Peso_Name,2,1,"[$ARS]\\ #,##0.00;\\-[$ARS]\\ #,##0.00")
FCUR(Argentina_Peso_Name,3,1,"[$ARS]\\ #,##0.00;[Red]\\-[$ARS]\\ #,##0.00")

FCUR(Colombia_Peso_Name,0,0,"[$COP]\\ #,##0")
FCUR(Colombia_Peso_Name,1,0,"[$COP]\\ #,##0;[Red][$COP]\\ #,##0")
FCUR(Colombia_Peso_Name,2,0,"[$COP]\\ #,##0;\\-[$COP]\\ #,##0")
FCUR(Colombia_Peso_Name,3,0,"[$COP]\\ #,##0;[Red]\\-[$COP]\\ #,##0")
FCUR(Colombia_Peso_Name,0,1,"[$COP]\\ #,##0.00")
FCUR(Colombia_Peso_Name,1,1,"[$COP]\\ #,##0.00;[Red][$COP]\\ #,##0.00")
FCUR(Colombia_Peso_Name,2,1,"[$COP]\\ #,##0.00;\\-[$COP]\\ #,##0.00")
FCUR(Colombia_Peso_Name,3,1,"[$COP]\\ #,##0.00;[Red]\\-[$COP]\\ #,##0.00")

FCUR(Guatemala_Quetzal_Name,0,0,"[$GTQ]\\ #,##0")
FCUR(Guatemala_Quetzal_Name,1,0,"[$GTQ]\\ #,##0;[Red][$GTQ]\\ #,##0")
FCUR(Guatemala_Quetzal_Name,2,0,"[$GTQ]\\ #,##0;\\-[$GTQ]\\ #,##0")
FCUR(Guatemala_Quetzal_Name,3,0,"[$GTQ]\\ #,##0;[Red]\\-[$GTQ]\\ #,##0")
FCUR(Guatemala_Quetzal_Name,0,1,"[$GTQ]\\ #,##0.00")
FCUR(Guatemala_Quetzal_Name,1,1,"[$GTQ]\\ #,##0.00;[Red][$GTQ]\\ #,##0.00")
FCUR(Guatemala_Quetzal_Name,2,1,"[$GTQ]\\ #,##0.00;\\-[$GTQ]\\ #,##0.00")
FCUR(Guatemala_Quetzal_Name,3,1,"[$GTQ]\\ #,##0.00;[Red]\\-[$GTQ]\\ #,##0.00")

FCUR(Bolivia_Boliviano_Name,0,0,"[$BOB]\\ #,##0")
FCUR(Bolivia_Boliviano_Name,1,0,"[$BOB]\\ #,##0;[Red][$BOB]\\ #,##0")
FCUR(Bolivia_Boliviano_Name,2,0,"[$BOB]\\ #,##0;\\-[$BOB]\\ #,##0")
FCUR(Bolivia_Boliviano_Name,3,0,"[$BOB]\\ #,##0;[Red]\\-[$BOB]\\ #,##0")
FCUR(Bolivia_Boliviano_Name,0,1,"[$BOB]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Name,1,1,"[$BOB]\\ #,##0.00;[Red][$BOB]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Name,2,1,"[$BOB]\\ #,##0.00;\\-[$BOB]\\ #,##0.00")
FCUR(Bolivia_Boliviano_Name,3,1,"[$BOB]\\ #,##0.00;[Red]\\-[$BOB]\\ #,##0.00")

FCUR(Chile_Peso_Name,0,0,"[$CLP]\\ #,##0")
FCUR(Chile_Peso_Name,1,0,"[$CLP]\\ #,##0;[Red][$CLP]\\ #,##0")
FCUR(Chile_Peso_Name,2,0,"[$CLP]\\ #,##0;\\-[$CLP]\\ #,##0")
FCUR(Chile_Peso_Name,3,0,"[$CLP]\\ #,##0;[Red]\\-[$CLP]\\ #,##0")
FCUR(Chile_Peso_Name,0,1,"[$CLP]\\ #,##0.00")
FCUR(Chile_Peso_Name,1,1,"[$CLP]\\ #,##0.00;[Red][$CLP]\\ #,##0.00")
FCUR(Chile_Peso_Name,2,1,"[$CLP]\\ #,##0.00;\\-[$CLP]\\ #,##0.00")
FCUR(Chile_Peso_Name,3,1,"[$CLP]\\ #,##0.00;[Red]\\-[$CLP]\\ #,##0.00")

FCUR(DominicanRepublic_Peso_Name,0,0,"[$DOP]\\ #,##0")
FCUR(DominicanRepublic_Peso_Name,1,0,"[$DOP]\\ #,##0;[Red][$DOP]\\ #,##0")
FCUR(DominicanRepublic_Peso_Name,2,0,"[$DOP]\\ #,##0;\\-[$DOP]\\ #,##0")
FCUR(DominicanRepublic_Peso_Name,3,0,"[$DOP]\\ #,##0;[Red]\\-[$DOP]\\ #,##0")
FCUR(DominicanRepublic_Peso_Name,0,1,"[$DOP]\\ #,##0.00")
FCUR(DominicanRepublic_Peso_Name,1,1,"[$DOP]\\ #,##0.00;[Red][$DOP]\\ #,##0.00")
FCUR(DominicanRepublic_Peso_Name,2,1,"[$DOP]\\ #,##0.00;\\-[$DOP]\\ #,##0.00")
FCUR(DominicanRepublic_Peso_Name,3,1,"[$DOP]\\ #,##0.00;[Red]\\-[$DOP]\\ #,##0.00")

FCUR(Ecuador_Sucre_Name,0,0,"[$ECS]\\ #,##0")
FCUR(Ecuador_Sucre_Name,1,0,"[$ECS]\\ #,##0;[Red][$ECS]\\ #,##0")
FCUR(Ecuador_Sucre_Name,2,0,"[$ECS]\\ #,##0;\\-[$ECS]\\ #,##0")
FCUR(Ecuador_Sucre_Name,3,0,"[$ECS]\\ #,##0;[Red]\\-[$ECS]\\ #,##0")
FCUR(Ecuador_Sucre_Name,0,1,"[$ECS]\\ #,##0.00")
FCUR(Ecuador_Sucre_Name,1,1,"[$ECS]\\ #,##0.00;[Red][$ECS]\\ #,##0.00")
FCUR(Ecuador_Sucre_Name,2,1,"[$ECS]\\ #,##0.00;\\-[$ECS]\\ #,##0.00")
FCUR(Ecuador_Sucre_Name,3,1,"[$ECS]\\ #,##0.00;[Red]\\-[$ECS]\\ #,##0.00")

FCUR(Mexico_Peso_Name,0,0,"[$MXN]\\ #,##0")
FCUR(Mexico_Peso_Name,1,0,"[$MXN]\\ #,##0;[Red][$MXN]\\ #,##0")
FCUR(Mexico_Peso_Name,2,0,"[$MXN]\\ #,##0;\\-[$MXN]\\ #,##0")
FCUR(Mexico_Peso_Name,3,0,"[$MXN]\\ #,##0;[Red]\\-[$MXN]\\ #,##0")
FCUR(Mexico_Peso_Name,0,1,"[$MXN]\\ #,##0.00")
FCUR(Mexico_Peso_Name,1,1,"[$MXN]\\ #,##0.00;[Red][$MXN]\\ #,##0.00")
FCUR(Mexico_Peso_Name,2,1,"[$MXN]\\ #,##0.00;\\-[$MXN]\\ #,##0.00")
FCUR(Mexico_Peso_Name,3,1,"[$MXN]\\ #,##0.00;[Red]\\-[$MXN]\\ #,##0.00")

FCUR(Peru_NuevoSol_Name,0,0,"[$PEN]\\ #,##0")
FCUR(Peru_NuevoSol_Name,1,0,"[$PEN]\\ #,##0;[Red][$PEN]\\ #,##0")
FCUR(Peru_NuevoSol_Name,2,0,"[$PEN]\\ #,##0;\\-[$PEN]\\ #,##0")
FCUR(Peru_NuevoSol_Name,3,0,"[$PEN]\\ #,##0;[Red]\\-[$PEN]\\ #,##0")
FCUR(Peru_NuevoSol_Name,0,1,"[$PEN]\\ #,##0.00")
FCUR(Peru_NuevoSol_Name,1,1,"[$PEN]\\ #,##0.00;[Red][$PEN]\\ #,##0.00")
FCUR(Peru_NuevoSol_Name,2,1,"[$PEN]\\ #,##0.00;\\-[$PEN]\\ #,##0.00")
FCUR(Peru_NuevoSol_Name,3,1,"[$PEN]\\ #,##0.00;[Red]\\-[$PEN]\\ #,##0.00")

FCUR(Paraguay_Guarani_Name,0,0,"[$PYG]\\ #,##0")
FCUR(Paraguay_Guarani_Name,1,0,"[$PYG]\\ #,##0;[Red][$PYG]\\ #,##0")
FCUR(Paraguay_Guarani_Name,2,0,"[$PYG]\\ #,##0;\\-[$PYG]\\ #,##0")
FCUR(Paraguay_Guarani_Name,3,0,"[$PYG]\\ #,##0;[Red]\\-[$PYG]\\ #,##0")
FCUR(Paraguay_Guarani_Name,0,1,"[$PYG]\\ #,##0.00")
FCUR(Paraguay_Guarani_Name,1,1,"[$PYG]\\ #,##0.00;[Red][$PYG]\\ #,##0.00")
FCUR(Paraguay_Guarani_Name,2,1,"[$PYG]\\ #,##0.00;\\-[$PYG]\\ #,##0.00")
FCUR(Paraguay_Guarani_Name,3,1,"[$PYG]\\ #,##0.00;[Red]\\-[$PYG]\\ #,##0.00")

FCUR(ElSalvador_Colon_Name,0,0,"[$SVC]\\ #,##0")
FCUR(ElSalvador_Colon_Name,1,0,"[$SVC]\\ #,##0;[Red][$SVC]\\ #,##0")
FCUR(ElSalvador_Colon_Name,2,0,"[$SVC]\\ #,##0;\\-[$SVC]\\ #,##0")
FCUR(ElSalvador_Colon_Name,3,0,"[$SVC]\\ #,##0;[Red]\\-[$SVC]\\ #,##0")
FCUR(ElSalvador_Colon_Name,0,1,"[$SVC]\\ #,##0.00")
FCUR(ElSalvador_Colon_Name,1,1,"[$SVC]\\ #,##0.00;[Red][$SVC]\\ #,##0.00")
FCUR(ElSalvador_Colon_Name,2,1,"[$SVC]\\ #,##0.00;\\-[$SVC]\\ #,##0.00")
FCUR(ElSalvador_Colon_Name,3,1,"[$SVC]\\ #,##0.00;[Red]\\-[$SVC]\\ #,##0.00")

FCUR(TrinidadTobago_Dollar_Name,0,0,"[$TTD]\\ #,##0")
FCUR(TrinidadTobago_Dollar_Name,1,0,"[$TTD]\\ #,##0;[Red][$TTD]\\ #,##0")
FCUR(TrinidadTobago_Dollar_Name,2,0,"[$TTD]\\ #,##0;\\-[$TTD]\\ #,##0")
FCUR(TrinidadTobago_Dollar_Name,3,0,"[$TTD]\\ #,##0;[Red]\\-[$TTD]\\ #,##0")
FCUR(TrinidadTobago_Dollar_Name,0,1,"[$TTD]\\ #,##0.00")
FCUR(TrinidadTobago_Dollar_Name,1,1,"[$TTD]\\ #,##0.00;[Red][$TTD]\\ #,##0.00")
FCUR(TrinidadTobago_Dollar_Name,2,1,"[$TTD]\\ #,##0.00;\\-[$TTD]\\ #,##0.00")
FCUR(TrinidadTobago_Dollar_Name,3,1,"[$TTD]\\ #,##0.00;[Red]\\-[$TTD]\\ #,##0.00")

FCUR(Honduras_Lempira_Name,0,0,"[$HNL]\\ #,##0")
FCUR(Honduras_Lempira_Name,1,0,"[$HNL]\\ #,##0;[Red][$HNL]\\ #,##0")
FCUR(Honduras_Lempira_Name,2,0,"[$HNL]\\ #,##0;\\-[$HNL]\\ #,##0")
FCUR(Honduras_Lempira_Name,3,0,"[$HNL]\\ #,##0;[Red]\\-[$HNL]\\ #,##0")
FCUR(Honduras_Lempira_Name,0,1,"[$HNL]\\ #,##0.00")
FCUR(Honduras_Lempira_Name,1,1,"[$HNL]\\ #,##0.00;[Red][$HNL]\\ #,##0.00")
FCUR(Honduras_Lempira_Name,2,1,"[$HNL]\\ #,##0.00;\\-[$HNL]\\ #,##0.00")
FCUR(Honduras_Lempira_Name,3,1,"[$HNL]\\ #,##0.00;[Red]\\-[$HNL]\\ #,##0.00")

FCUR(Nicaragua_Cordoba_Name,0,0,"[$NIO]\\ #,##0")
FCUR(Nicaragua_Cordoba_Name,1,0,"[$NIO]\\ #,##0;[Red][$NIO]\\ #,##0")
FCUR(Nicaragua_Cordoba_Name,2,0,"[$NIO]\\ #,##0;\\-[$NIO]\\ #,##0")
FCUR(Nicaragua_Cordoba_Name,3,0,"[$NIO]\\ #,##0;[Red]\\-[$NIO]\\ #,##0")
FCUR(Nicaragua_Cordoba_Name,0,1,"[$NIO]\\ #,##0.00")
FCUR(Nicaragua_Cordoba_Name,1,1,"[$NIO]\\ #,##0.00;[Red][$NIO]\\ #,##0.00")
FCUR(Nicaragua_Cordoba_Name,2,1,"[$NIO]\\ #,##0.00;\\-[$NIO]\\ #,##0.00")
FCUR(Nicaragua_Cordoba_Name,3,1,"[$NIO]\\ #,##0.00;[Red]\\-[$NIO]\\ #,##0.00")

FCUR(Venezuela_Bolivar_Name,0,0,"[$VEB]\\ #,##0")
FCUR(Venezuela_Bolivar_Name,1,0,"[$VEB]\\ #,##0;[Red][$VEB]\\ #,##0")
FCUR(Venezuela_Bolivar_Name,2,0,"[$VEB]\\ #,##0;\\-[$VEB]\\ #,##0")
FCUR(Venezuela_Bolivar_Name,3,0,"[$VEB]\\ #,##0;[Red]\\-[$VEB]\\ #,##0")
FCUR(Venezuela_Bolivar_Name,0,1,"[$VEB]\\ #,##0.00")
FCUR(Venezuela_Bolivar_Name,1,1,"[$VEB]\\ #,##0.00;[Red][$VEB]\\ #,##0.00")
FCUR(Venezuela_Bolivar_Name,2,1,"[$VEB]\\ #,##0.00;\\-[$VEB]\\ #,##0.00")
FCUR(Venezuela_Bolivar_Name,3,1,"[$VEB]\\ #,##0.00;[Red]\\-[$VEB]\\ #,##0.00")

FCUR(Panama_Balboa_Name,0,0,"[$PAB]\\ #,##0")
FCUR(Panama_Balboa_Name,1,0,"[$PAB]\\ #,##0;[Red][$PAB]\\ #,##0")
FCUR(Panama_Balboa_Name,2,0,"[$PAB]\\ #,##0;\\-[$PAB]\\ #,##0")
FCUR(Panama_Balboa_Name,3,0,"[$PAB]\\ #,##0;[Red]\\-[$PAB]\\ #,##0")
FCUR(Panama_Balboa_Name,0,1,"[$PAB]\\ #,##0.00")
FCUR(Panama_Balboa_Name,1,1,"[$PAB]\\ #,##0.00;[Red][$PAB]\\ #,##0.00")
FCUR(Panama_Balboa_Name,2,1,"[$PAB]\\ #,##0.00;\\-[$PAB]\\ #,##0.00")
FCUR(Panama_Balboa_Name,3,1,"[$PAB]\\ #,##0.00;[Red]\\-[$PAB]\\ #,##0.00")

FCUR(CostaRica_Colon,0,0,"[$CRC]\\ #,##0")
FCUR(CostaRica_Colon,1,0,"[$CRC]\\ #,##0;[Red][$CRC]\\ #,##0")
FCUR(CostaRica_Colon,2,0,"[$CRC]\\ #,##0;\\-[$CRC]\\ #,##0")
FCUR(CostaRica_Colon,3,0,"[$CRC]\\ #,##0;[Red]\\-[$CRC]\\ #,##0")
FCUR(CostaRica_Colon,0,1,"[$CRC]\\ #,##0.00")
FCUR(CostaRica_Colon,1,1,"[$CRC]\\ #,##0.00;[Red][$CRC]\\ #,##0.00")
FCUR(CostaRica_Colon,2,1,"[$CRC]\\ #,##0.00;\\-[$CRC]\\ #,##0.00")
FCUR(CostaRica_Colon,3,1,"[$CRC]\\ #,##0.00;[Red]\\-[$CRC]\\ #,##0.00")

FCUR(Haiti_Gourde_Name,0,0,"[$HTG]\\ #,##0")
FCUR(Haiti_Gourde_Name,1,0,"[$HTG]\\ #,##0;[Red][$HTG]\\ #,##0")
FCUR(Haiti_Gourde_Name,2,0,"[$HTG]\\ #,##0;\\-[$HTG]\\ #,##0")
FCUR(Haiti_Gourde_Name,3,0,"[$HTG]\\ #,##0;[Red]\\-[$HTG]\\ #,##0")
FCUR(Haiti_Gourde_Name,0,1,"[$HTG]\\ #,##0.00")
FCUR(Haiti_Gourde_Name,1,1,"[$HTG]\\ #,##0.00;[Red][$HTG]\\ #,##0.00")
FCUR(Haiti_Gourde_Name,2,1,"[$HTG]\\ #,##0.00;\\-[$HTG]\\ #,##0.00")
FCUR(Haiti_Gourde_Name,3,1,"[$HTG]\\ #,##0.00;[Red]\\-[$HTG]\\ #,##0.00")

FCUR(Jamaica_Dollar_Name,0,0,"[$JMD]\\ #,##0")
FCUR(Jamaica_Dollar_Name,1,0,"[$JMD]\\ #,##0;[Red][$JMD]\\ #,##0")
FCUR(Jamaica_Dollar_Name,2,0,"[$JMD]\\ #,##0;\\-[$JMD]\\ #,##0")
FCUR(Jamaica_Dollar_Name,3,0,"[$JMD]\\ #,##0;[Red]\\-[$JMD]\\ #,##0")
FCUR(Jamaica_Dollar_Name,0,1,"[$JMD]\\ #,##0.00")
FCUR(Jamaica_Dollar_Name,1,1,"[$JMD]\\ #,##0.00;[Red][$JMD]\\ #,##0.00")
FCUR(Jamaica_Dollar_Name,2,1,"[$JMD]\\ #,##0.00;\\-[$JMD]\\ #,##0.00")
FCUR(Jamaica_Dollar_Name,3,1,"[$JMD]\\ #,##0.00;[Red]\\-[$JMD]\\ #,##0.00")

FCUR(UnitedKingdom_Pound_Name,0,0,"[$GBP]\\ #,##0")
FCUR(UnitedKingdom_Pound_Name,1,0,"[$GBP]\\ #,##0;[Red][$GBP]\\ #,##0")
FCUR(UnitedKingdom_Pound_Name,2,0,"[$GBP]\\ #,##0;\\-[$GBP]\\ #,##0")
FCUR(UnitedKingdom_Pound_Name,3,0,"[$GBP]\\ #,##0;[Red]\\-[$GBP]\\ #,##0")
FCUR(UnitedKingdom_Pound_Name,0,1,"[$GBP]\\ #,##0.00")
FCUR(UnitedKingdom_Pound_Name,1,1,"[$GBP]\\ #,##0.00;[Red][$GBP]\\ #,##0.00")
FCUR(UnitedKingdom_Pound_Name,2,1,"[$GBP]\\ #,##0.00;\\-[$GBP]\\ #,##0.00")
FCUR(UnitedKingdom_Pound_Name,3,1,"[$GBP]\\ #,##0.00;[Red]\\-[$GBP]\\ #,##0.00")

FCUR(UnitedStates_Dollar_Name,0,0,"[$USD]\\ #,##0")
FCUR(UnitedStates_Dollar_Name,1,0,"[$USD]\\ #,##0;[Red][$USD]\\ #,##0")
FCUR(UnitedStates_Dollar_Name,2,0,"[$USD]\\ #,##0;\\-[$USD]\\ #,##0")
FCUR(UnitedStates_Dollar_Name,3,0,"[$USD]\\ #,##0;[Red]\\-[$USD]\\ #,##0")
FCUR(UnitedStates_Dollar_Name,0,1,"[$USD]\\ #,##0.00")
FCUR(UnitedStates_Dollar_Name,1,1,"[$USD]\\ #,##0.00;[Red][$USD]\\ #,##0.00")
FCUR(UnitedStates_Dollar_Name,2,1,"[$USD]\\ #,##0.00;\\-[$USD]\\ #,##0.00")
FCUR(UnitedStates_Dollar_Name,3,1,"[$USD]\\ #,##0.00;[Red]\\-[$USD]\\ #,##0.00")

FCUR(Euro_Name,0,0,"[$EUR]\\ #,##0")
FCUR(Euro_Name,1,0,"[$EUR]\\ #,##0;[Red][$EUR]\\ #,##0")
FCUR(Euro_Name,2,0,"[$EUR]\\ #,##0;\\-[$EUR]\\ #,##0")
FCUR(Euro_Name,3,0,"[$EUR]\\ #,##0;[Red]\\-[$EUR]\\ #,##0")
FCUR(Euro_Name,0,1,"[$EUR]\\ #,##0.00")
FCUR(Euro_Name,1,1,"[$EUR]\\ #,##0.00;[Red][$EUR]\\ #,##0.00")
FCUR(Euro_Name,2,1,"[$EUR]\\ #,##0.00;\\-[$EUR]\\ #,##0.00")
FCUR(Euro_Name,3,1,"[$EUR]\\ #,##0.00;[Red]\\-[$EUR]\\ #,##0.00")

/* Accounting ; 53 symbols * 2 decimal place settings */
FACC(Brazil_Real,0,"_-[$R$-416]\\ * #,##0_-;\\-[$R$-416]\\ * #,##0_-;_-[$R$-416]\\ * \"-\"_-;_-@_-")
FACC(Brazil_Real,1,"_-[$R$-416]\\ * #,##0.00_-;\\-[$R$-416]\\ * #,##0.00_-;_-[$R$-416]\\ * \"-\"\??_-;_-@_-")

FACC(Uruguay_Peso,0,"_ [$$U-380A]\\ * #,##0_ ;_ [$$U-380A]\\ * \\-#,##0_ ;_ [$$U-380A]\\ * \"-\"_ ;_ @_ ")
FACC(Uruguay_Peso,1,"_ [$$U-380A]\\ * #,##0.00_ ;_ [$$U-380A]\\ * \\-#,##0.00_ ;_ [$$U-380A]\\ * \"-\"\??_ ;_ @_ ")

FACC(Argentina_Peso,0,"_ [$$-2C0A]\\ * #,##0_ ;_ [$$-2C0A]\\ * \\-#,##0_ ;_ [$$-2C0A]\\ * \"-\"_ ;_ @_ ")
FACC(Argentina_Peso,1,"_ [$$-2C0A]\\ * #,##0.00_ ;_ [$$-2C0A]\\ * \\-#,##0.00_ ;_ [$$-2C0A]\\ * \"-\"\??_ ;_ @_ ")

FACC(Colombia_Peso,0,"_ [$$-240A]\\ * #,##0_ ;_ [$$-240A]\\ * \\-#,##0_ ;_ [$$-240A]\\ * \"-\"_ ;_ @_ ")
FACC(Colombia_Peso,1,"_ [$$-240A]\\ * #,##0.00_ ;_ [$$-240A]\\ * \\-#,##0.00_ ;_ [$$-240A]\\ * \"-\"\??_ ;_ @_ ")

FACC(Guatemala_Quetzal,0,"_-[$Q-100A]* #,##0_ ;_-[$Q-100A]* \\-#,##0\\ ;_-[$Q-100A]* \"-\"_ ;_-@_ ")
FACC(Guatemala_Quetzal,1,"_-[$Q-100A]* #,##0.00_ ;_-[$Q-100A]* \\-#,##0.00\\ ;_-[$Q-100A]* \"-\"\??_ ;_-@_ ")

FACC(Bolivia_Boliviano_Quechua,0,"_ [$$b-46B]\\ * #,##0_ ;_ [$$b-46B]\\ * \\-#,##0_ ;_ [$$b-46B]\\ * \"-\"_ ;_ @_ ")
FACC(Bolivia_Boliviano_Quechua,1,"_ [$$b-46B]\\ * #,##0.00_ ;_ [$$b-46B]\\ * \\-#,##0.00_ ;_ [$$b-46B]\\ * \"-\"\??_ ;_ @_ ")

FACC(Bolivia_Boliviano_Spanish,0,"_ [$$b-400A]\\ * #,##0_ ;_ [$$b-400A]\\ * \\-#,##0_ ;_ [$$b-400A]\\ * \"-\"_ ;_ @_ ")
FACC(Bolivia_Boliviano_Spanish,1,"_ [$$b-400A]\\ * #,##0.00_ ;_ [$$b-400A]\\ * \\-#,##0.00_ ;_ [$$b-400A]\\ * \"-\"\??_ ;_ @_ ")

FACC(Chile_Peso,0,"_-[$$-340A]\\ * #,##0_-;\\-[$$-340A]\\ * #,##0_-;_-[$$-340A]\\ * \"-\"_-;_-@_-")
FACC(Chile_Peso,1,"_-[$$-340A]\\ * #,##0.00_-;\\-[$$-340A]\\ * #,##0.00_-;_-[$$-340A]\\ * \"-\"\??_-;_-@_-")

FACC(DominicanRepublic_Peso,0,"_-[$RD$-1C0A]* #,##0_ ;_-[$RD$-1C0A]* \\-#,##0\\ ;_-[$RD$-1C0A]* \"-\"_ ;_-@_ ")
FACC(DominicanRepublic_Peso,1,"_-[$RD$-1C0A]* #,##0.00_ ;_-[$RD$-1C0A]* \\-#,##0.00\\ ;_-[$RD$-1C0A]* \"-\"\??_ ;_-@_ ")

FACC(Ecuador_Sucre_Quechua,0,"_ [$$-86B]\\ * #,##0_ ;_ [$$-86B]\\ * \\-#,##0_ ;_ [$$-86B]\\ * \"-\"_ ;_ @_ ")
FACC(Ecuador_Sucre_Quechua,1,"_ [$$-86B]\\ * #,##0.00_ ;_ [$$-86B]\\ * \\-#,##0.00_ ;_ [$$-86B]\\ * \"-\"\??_ ;_ @_ ")

FACC(Ecuador_Sucre_Spanish,0,"_ [$$-300A]\\ * #,##0_ ;_ [$$-300A]\\ * \\-#,##0_ ;_ [$$-300A]\\ * \"-\"_ ;_ @_ ")
FACC(Ecuador_Sucre_Spanish,1,"_ [$$-300A]\\ * #,##0.00_ ;_ [$$-300A]\\ * \\-#,##0.00_ ;_ [$$-300A]\\ * \"-\"\??_ ;_ @_ ")

FACC(Mexico_Peso,0,"_-[$$-80A]* #,##0_-;\\-[$$-80A]* #,##0_-;_-[$$-80A]* \"-\"_-;_-@_-")
FACC(Mexico_Peso,1,"_-[$$-80A]* #,##0.00_-;\\-[$$-80A]* #,##0.00_-;_-[$$-80A]* \"-\"\??_-;_-@_-")

FACC(Peru_NuevoSol_Quechua,0,"_-[$S/.-C6B]\\ * #,##0_ ;_-[$S/.-C6B]\\ * \\-#,##0\\ ;_-[$S/.-C6B]\\ * \"-\"_ ;_-@_ ")
FACC(Peru_NuevoSol_Quechua,1,"_-[$S/.-C6B]\\ * #,##0.00_ ;_-[$S/.-C6B]\\ * \\-#,##0.00\\ ;_-[$S/.-C6B]\\ * \"-\"\??_ ;_-@_ ")

FACC(Peru_NuevoSol_Spanish,0,"_-[$S/.-280A]\\ * #,##0_ ;_-[$S/.-280A]\\ * \\-#,##0\\ ;_-[$S/.-280A]\\ * \"-\"_ ;_-@_ ")
FACC(Peru_NuevoSol_Spanish,1,"_-[$S/.-280A]\\ * #,##0.00_ ;_-[$S/.-280A]\\ * \\-#,##0.00\\ ;_-[$S/.-280A]\\ * \"-\"\??_ ;_-@_ ")

FACC(Paraguay_Guarani,0,"_ [$Gs-3C0A]\\ * #,##0_ ;_ [$Gs-3C0A]\\ * \\-#,##0_ ;_ [$Gs-3C0A]\\ * \"-\"_ ;_ @_ ")
FACC(Paraguay_Guarani,1,"_ [$Gs-3C0A]\\ * #,##0.00_ ;_ [$Gs-3C0A]\\ * \\-#,##0.00_ ;_ [$Gs-3C0A]\\ * \"-\"\??_ ;_ @_ ")

FACC(ElSalvador_Colon,0,"_-[$$-440A]* #,##0_ ;_-[$$-440A]* \\-#,##0\\ ;_-[$$-440A]* \"-\"_ ;_-@_ ")
FACC(ElSalvador_Colon,1,"_-[$$-440A]* #,##0.00_ ;_-[$$-440A]* \\-#,##0.00\\ ;_-[$$-440A]* \"-\"\??_ ;_-@_ ")

FACC(TrinidadTobago_Dollar,0,"_-[$TT$-2C09]* #,##0_ ;_-[$TT$-2C09]* \\-#,##0\\ ;_-[$TT$-2C09]* \"-\"_ ;_-@_ ")
FACC(TrinidadTobago_Dollar,1,"_-[$TT$-2C09]* #,##0.00_ ;_-[$TT$-2C09]* \\-#,##0.00\\ ;_-[$TT$-2C09]* \"-\"\??_ ;_-@_ ")

FACC(Honduras_Lempira,0,"_-[$L.-480A]\\ * #,##0_ ;_-[$L.-480A]\\ * \\-#,##0\\ ;_-[$L.-480A]\\ * \"-\"_ ;_-@_ ")
FACC(Honduras_Lempira,1,"_-[$L.-480A]\\ * #,##0.00_ ;_-[$L.-480A]\\ * \\-#,##0.00\\ ;_-[$L.-480A]\\ * \"-\"\??_ ;_-@_ ")

FACC(Nicaragua_Cordoba,0,"_ [$C$-4C0A]\\ * #,##0_ ;_ [$C$-4C0A]\\ * \\-#,##0_ ;_ [$C$-4C0A]\\ * \"-\"_ ;_ @_ ")
FACC(Nicaragua_Cordoba,1,"_ [$C$-4C0A]\\ * #,##0.00_ ;_ [$C$-4C0A]\\ * \\-#,##0.00_ ;_ [$C$-4C0A]\\ * \"-\"\??_ ;_ @_ ")

FACC(Venezuela_Bolivar,0,"_-[$Bs-200A]\\ * #,##0_ ;_-[$Bs-200A]\\ * \\-#,##0\\ ;_-[$Bs-200A]\\ * \"-\"_ ;_-@_ ")
FACC(Venezuela_Bolivar,1,"_-[$Bs-200A]\\ * #,##0.00_ ;_-[$Bs-200A]\\ * \\-#,##0.00\\ ;_-[$Bs-200A]\\ * \"-\"\??_ ;_-@_ ")

FACC(Panama_Balboa,0,"_ [$B/.-180A]\\ * #,##0_ ;_ [$B/.-180A]\\ * \\-#,##0_ ;_ [$B/.-180A]\\ * \"-\"_ ;_ @_ ")
FACC(Panama_Balboa,1,"_ [$B/.-180A]\\ * #,##0.00_ ;_ [$B/.-180A]\\ * \\-#,##0.00_ ;_ [$B/.-180A]\\ * \"-\"\??_ ;_ @_ ")

FACC(CostaRica_Colon,0,"_-[$\xe2\x82\xa1-140A]* #,##0_ ;_-[$\xe2\x82\xa1-140A]* \\-#,##0\\ ;_-[$\xe2\x82\xa1-140A]* \"-\"_ ;_-@_ ")
FACC(CostaRica_Colon,1,"_-[$\xe2\x82\xa1-140A]* #,##0.00_ ;_-[$\xe2\x82\xa1-140A]* \\-#,##0.00\\ ;_-[$\xe2\x82\xa1-140A]* \"-\"\??_ ;_-@_ ")

FACC(Jamaica_Dollar,0,"_-[$J$-2009]* #,##0_-;\\-[$J$-2009]* #,##0_-;_-[$J$-2009]* \"-\"_-;_-@_-")
FACC(Jamaica_Dollar,1,"_-[$J$-2009]* #,##0.00_-;\\-[$J$-2009]* #,##0.00_-;_-[$J$-2009]* \"-\"\??_-;_-@_-")

FACC(FrenchCanada_Dollar,0,"_ * #,##0_ \\ [$$-C0C]_ ;_ * \\-#,##0\\ \\ [$$-C0C]_ ;_ * \"-\"_ \\ [$$-C0C]_ ;_ @_ ")
FACC(FrenchCanada_Dollar,1,"_ * #,##0.00_ \\ [$$-C0C]_ ;_ * \\-#,##0.00\\ \\ [$$-C0C]_ ;_ * \"-\"\??_ \\ [$$-C0C]_ ;_ @_ ")

FACC(Sterling,0,"_-\"\xc2\xa3\"* #,##0_-;\\-\"\xc2\xa3\"* #,##0_-;_-\"\xc2\xa3\"* \"-\"_-;_-@_-")
FACC(Sterling,1,"_-\"\xc2\xa3\"* #,##0.00_-;\\-\"\xc2\xa3\"* #,##0.00_-;_-\"\xc2\xa3\"* \"-\"\??_-;_-@_-")

FACC(UnitedKingdom_Pound,0,"_-[$\xc2\xa3-809]* #,##0_-;\\-[$\xc2\xa3-809]* #,##0_-;_-[$\xc2\xa3-809]* \"-\"_-;_-@_-")
FACC(UnitedKingdom_Pound,1,"_-[$\xc2\xa3-809]* #,##0.00_-;\\-[$\xc2\xa3-809]* #,##0.00_-;_-[$\xc2\xa3-809]* \"-\"\??_-;_-@_-")

FACC(UnitedStates_Dollar,0,"_-[$$-409]* #,##0_ ;_-[$$-409]* \\-#,##0\\ ;_-[$$-409]* \"-\"_ ;_-@_ ")
FACC(UnitedStates_Dollar,1,"_-[$$-409]* #,##0.00_ ;_-[$$-409]* \\-#,##0.00\\ ;_-[$$-409]* \"-\"\??_ ;_-@_ ")

FACC(Euro,0,"_-[$\xe2\x82\xac-2]\\ * #,##0_-;\\-[$\xe2\x82\xac-2]\\ * #,##0_-;_-[$\xe2\x82\xac-2]\\ * \"-\"_-;_-@_-")
FACC(Euro,1,"_-[$\xe2\x82\xac-2]\\ * #,##0.00_-;\\-[$\xe2\x82\xac-2]\\ * #,##0.00_-;_-[$\xe2\x82\xac-2]\\ * \"-\"\??_-;_-@_-")

FACC(Euro_Alt,0,"_-* #,##0\\ [$\xe2\x82\xac-1]_-;\\-* #,##0\\ [$\xe2\x82\xac-1]_-;_-* \"-\"\\ [$\xe2\x82\xac-1]_-;_-@_-")
FACC(Euro_Alt,1,"_-* #,##0.00\\ [$\xe2\x82\xac-1]_-;\\-* #,##0.00\\ [$\xe2\x82\xac-1]_-;_-* \"-\"\??\\ [$\xe2\x82\xac-1]_-;_-@_-")

FACC(Brazil_Real_Name,0,"_-[$BRL]\\ * #,##0_-;\\-[$BRL]\\ * #,##0_-;_-[$BRL]\\ * \"-\"_-;_-@_-")
FACC(Brazil_Real_Name,1,"_-[$BRL]\\ * #,##0.00_-;\\-[$BRL]\\ * #,##0.00_-;_-[$BRL]\\ * \"-\"\??_-;_-@_-")

FACC(Uruguay_Peso_Name,0,"_-[$UYU]\\ * #,##0_-;\\-[$UYU]\\ * #,##0_-;_-[$UYU]\\ * \"-\"_-;_-@_-")
FACC(Uruguay_Peso_Name,1,"_-[$UYU]\\ * #,##0.00_-;\\-[$UYU]\\ * #,##0.00_-;_-[$UYU]\\ * \"-\"\??_-;_-@_-")

FACC(Argentina_Peso_Name,0,"_-[$ARS]\\ * #,##0_-;\\-[$ARS]\\ * #,##0_-;_-[$ARS]\\ * \"-\"_-;_-@_-")
FACC(Argentina_Peso_Name,1,"_-[$ARS]\\ * #,##0.00_-;\\-[$ARS]\\ * #,##0.00_-;_-[$ARS]\\ * \"-\"\??_-;_-@_-")

FACC(Colombia_Peso_Name,0,"_-[$COP]\\ * #,##0_-;\\-[$COP]\\ * #,##0_-;_-[$COP]\\ * \"-\"_-;_-@_-")
FACC(Colombia_Peso_Name,1,"_-[$COP]\\ * #,##0.00_-;\\-[$COP]\\ * #,##0.00_-;_-[$COP]\\ * \"-\"\??_-;_-@_-")

FACC(Guatemala_Quetzal_Name,0,"_-[$GTQ]\\ * #,##0_-;\\-[$GTQ]\\ * #,##0_-;_-[$GTQ]\\ * \"-\"_-;_-@_-")
FACC(Guatemala_Quetzal_Name,1,"_-[$GTQ]\\ * #,##0.00_-;\\-[$GTQ]\\ * #,##0.00_-;_-[$GTQ]\\ * \"-\"\??_-;_-@_-")

FACC(Bolivia_Boliviano_Name,0,"_-[$BOB]\\ * #,##0_-;\\-[$BOB]\\ * #,##0_-;_-[$BOB]\\ * \"-\"_-;_-@_-")
FACC(Bolivia_Boliviano_Name,1,"_-[$BOB]\\ * #,##0.00_-;\\-[$BOB]\\ * #,##0.00_-;_-[$BOB]\\ * \"-\"\??_-;_-@_-")

FACC(Chile_Peso_Name,0,"_-[$CLP]\\ * #,##0_-;\\-[$CLP]\\ * #,##0_-;_-[$CLP]\\ * \"-\"_-;_-@_-")
FACC(Chile_Peso_Name,1,"_-[$CLP]\\ * #,##0.00_-;\\-[$CLP]\\ * #,##0.00_-;_-[$CLP]\\ * \"-\"\??_-;_-@_-")

FACC(DominicanRepublic_Peso_Name,0,"_-[$DOP]\\ * #,##0_-;\\-[$DOP]\\ * #,##0_-;_-[$DOP]\\ * \"-\"_-;_-@_-")
FACC(DominicanRepublic_Peso_Name,1,"_-[$DOP]\\ * #,##0.00_-;\\-[$DOP]\\ * #,##0.00_-;_-[$DOP]\\ * \"-\"\??_-;_-@_-")

FACC(Ecuador_Sucre_Name,0,"_-[$ECS]\\ * #,##0_-;\\-[$ECS]\\ * #,##0_-;_-[$ECS]\\ * \"-\"_-;_-@_-")
FACC(Ecuador_Sucre_Name,1,"_-[$ECS]\\ * #,##0.00_-;\\-[$ECS]\\ * #,##0.00_-;_-[$ECS]\\ * \"-\"\??_-;_-@_-")

FACC(Mexico_Peso_Name,0,"_-[$MXN]\\ * #,##0_-;\\-[$MXN]\\ * #,##0_-;_-[$MXN]\\ * \"-\"_-;_-@_-")
FACC(Mexico_Peso_Name,1,"_-[$MXN]\\ * #,##0.00_-;\\-[$MXN]\\ * #,##0.00_-;_-[$MXN]\\ * \"-\"\??_-;_-@_-")

FACC(Peru_NuevoSol_Name,0,"_-[$PEN]\\ * #,##0_-;\\-[$PEN]\\ * #,##0_-;_-[$PEN]\\ * \"-\"_-;_-@_-")
FACC(Peru_NuevoSol_Name,1,"_-[$PEN]\\ * #,##0.00_-;\\-[$PEN]\\ * #,##0.00_-;_-[$PEN]\\ * \"-\"\??_-;_-@_-")

FACC(Paraguay_Guarani_Name,0,"_-[$PYG]\\ * #,##0_-;\\-[$PYG]\\ * #,##0_-;_-[$PYG]\\ * \"-\"_-;_-@_-")
FACC(Paraguay_Guarani_Name,1,"_-[$PYG]\\ * #,##0.00_-;\\-[$PYG]\\ * #,##0.00_-;_-[$PYG]\\ * \"-\"\??_-;_-@_-")

FACC(ElSalvador_Colon_Name,0,"_-[$SVC]\\ * #,##0_-;\\-[$SVC]\\ * #,##0_-;_-[$SVC]\\ * \"-\"_-;_-@_-")
FACC(ElSalvador_Colon_Name,1,"_-[$SVC]\\ * #,##0.00_-;\\-[$SVC]\\ * #,##0.00_-;_-[$SVC]\\ * \"-\"\??_-;_-@_-")

FACC(TrinidadTobago_Dollar_Name,0,"_-[$TTD]\\ * #,##0_-;\\-[$TTD]\\ * #,##0_-;_-[$TTD]\\ * \"-\"_-;_-@_-")
FACC(TrinidadTobago_Dollar_Name,1,"_-[$TTD]\\ * #,##0.00_-;\\-[$TTD]\\ * #,##0.00_-;_-[$TTD]\\ * \"-\"\??_-;_-@_-")

FACC(Honduras_Lempira_Name,0,"_-[$HNL]\\ * #,##0_-;\\-[$HNL]\\ * #,##0_-;_-[$HNL]\\ * \"-\"_-;_-@_-")
FACC(Honduras_Lempira_Name,1,"_-[$HNL]\\ * #,##0.00_-;\\-[$HNL]\\ * #,##0.00_-;_-[$HNL]\\ * \"-\"\??_-;_-@_-")

FACC(Nicaragua_Cordoba_Name,0,"_-[$NIO]\\ * #,##0_-;\\-[$NIO]\\ * #,##0_-;_-[$NIO]\\ * \"-\"_-;_-@_-")
FACC(Nicaragua_Cordoba_Name,1,"_-[$NIO]\\ * #,##0.00_-;\\-[$NIO]\\ * #,##0.00_-;_-[$NIO]\\ * \"-\"\??_-;_-@_-")

FACC(Venezuela_Bolivar_Name,0,"_-[$VEB]\\ * #,##0_-;\\-[$VEB]\\ * #,##0_-;_-[$VEB]\\ * \"-\"_-;_-@_-")
FACC(Venezuela_Bolivar_Name,1,"_-[$VEB]\\ * #,##0.00_-;\\-[$VEB]\\ * #,##0.00_-;_-[$VEB]\\ * \"-\"\??_-;_-@_-")

FACC(Panama_Balboa_Name,0,"_-[$PAB]\\ * #,##0_-;\\-[$PAB]\\ * #,##0_-;_-[$PAB]\\ * \"-\"_-;_-@_-")
FACC(Panama_Balboa_Name,1,"_-[$PAB]\\ * #,##0.00_-;\\-[$PAB]\\ * #,##0.00_-;_-[$PAB]\\ * \"-\"\??_-;_-@_-")

FACC(CostaRica_Colon_Name,0,"_-[$CRC]\\ * #,##0_-;\\-[$CRC]\\ * #,##0_-;_-[$CRC]\\ * \"-\"_-;_-@_-")
FACC(CostaRica_Colon_Name,1,"_-[$CRC]\\ * #,##0.00_-;\\-[$CRC]\\ * #,##0.00_-;_-[$CRC]\\ * \"-\"\??_-;_-@_-")

FACC(Haiti_Gourde_Name,0,"_-[$HTG]\\ * #,##0_-;\\-[$HTG]\\ * #,##0_-;_-[$HTG]\\ * \"-\"_-;_-@_-")
FACC(Haiti_Gourde_Name,1,"_-[$HTG]\\ * #,##0.00_-;\\-[$HTG]\\ * #,##0.00_-;_-[$HTG]\\ * \"-\"\??_-;_-@_-")

FACC(Jamaica_Dollar_Name,0,"_-[$JMD]\\ * #,##0_-;\\-[$JMD]\\ * #,##0_-;_-[$JMD]\\ * \"-\"_-;_-@_-")
FACC(Jamaica_Dollar_Name,1,"_-[$JMD]\\ * #,##0.00_-;\\-[$JMD]\\ * #,##0.00_-;_-[$JMD]\\ * \"-\"\??_-;_-@_-")

FACC(UnitedKingdom_Pound_Name,0,"_-[$GBP]\\ * #,##0_-;\\-[$GBP]\\ * #,##0_-;_-[$GBP]\\ * \"-\"_-;_-@_-")
FACC(UnitedKingdom_Pound_Name,1,"_-[$GBP]\\ * #,##0.00_-;\\-[$GBP]\\ * #,##0.00_-;_-[$GBP]\\ * \"-\"\??_-;_-@_-")

FACC(UnitedStates_Dollar_Name,0,"_-[$USD]\\ * #,##0_-;\\-[$USD]\\ * #,##0_-;_-[$USD]\\ * \"-\"_-;_-@_-")
FACC(UnitedStates_Dollar_Name,1,"_-[$USD]\\ * #,##0.00_-;\\-[$USD]\\ * #,##0.00_-;_-[$USD]\\ * \"-\"\??_-;_-@_-")

FACC(Euro_Name,0,"_-[$EUR]\\ * #,##0_-;\\-[$EUR]\\ * #,##0_-;_-[$EUR]\\ * \"-\"_-;_-@_-")
FACC(Euro_Name,1,"_-[$EUR]\\ * #,##0.00_-;\\-[$EUR]\\ * #,##0.00_-;_-[$EUR]\\ * \"-\"\??_-;_-@_-")

/* Date ; 27 countries with up to 15 formats each */
FDATE(Brazil,0,"d/m;@")
FDATE(Brazil,1,"d/m/yy;@")
FDATE(Brazil,2,"dd/mm/yy;@")
FDATE(Brazil,3,"[$-416]d\\-mmm;@")
FDATE(Brazil,4,"[$-416]d\\-mmm\\-yy;@")
FDATE(Brazil,5,"[$-416]dd\\-mmm\\-yy;@")
FDATE(Brazil,6,"[$-416]mmm\\-yy;@")
FDATE(Brazil,7,"[$-416]mmmm\\-yy;@")
FDATE(Brazil,8,"[$-416]d\\ \\ mmmm\\,\\ yyyy;@")
FDATE(Brazil,9,"[$-409]d/m/yy\\ h:mm\\ AM/PM;@")
FDATE(Brazil,10,"[$-416]d\\ \\ mmmm\\,\\ yyyy;@")

FDATE(Uruguay,0,"dd/mm/yyyy;@")
FDATE(Uruguay,1,"dd/mm/yy;@")
FDATE(Uruguay,2,"d/m/yy;@")
FDATE(Uruguay,3,"dd\\-mm\\-yy;@")
FDATE(Uruguay,4,"yyyy\\-mm\\-dd;@")
FDATE(Uruguay,5,"[$-380A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Uruguay,6,"[$-380A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Uruguay,7,"[$-380A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Argentina,0,"dd/mm/yyyy;@")
FDATE(Argentina,1,"dd/mm/yy;@")
FDATE(Argentina,2,"d/m/yy;@")
FDATE(Argentina,3,"dd\\-mm\\-yy;@")
FDATE(Argentina,4,"yyyy\\-mm\\-dd;@")
FDATE(Argentina,5,"[$-2C0A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Argentina,6,"[$-2C0A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Argentina,7,"[$-2C0A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Colombia,0,"dd/mm/yyyy;@")
FDATE(Colombia,1,"dd/mm/yy;@")
FDATE(Colombia,2,"d/mm/yyyy;@")
FDATE(Colombia,3,"d/m/yy;@")
FDATE(Colombia,4,"dd\\-mm\\-yy;@")
FDATE(Colombia,5,"yyyy\\-mm\\-dd;@")
FDATE(Colombia,6,"[$-240A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Colombia,7,"[$-240A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Colombia,8,"[$-240A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Guatemala,0,"dd/mm/yyyy;@")
FDATE(Guatemala,1,"dd/mm/yy;@")
FDATE(Guatemala,2,"d/mm/yyyy;@")
FDATE(Guatemala,3,"d/m/yy;@")
FDATE(Guatemala,4,"dd\\-mm\\-yy;@")
FDATE(Guatemala,5,"yyyy\\-mm\\-dd;@")
FDATE(Guatemala,6,"[$-100A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Guatemala,7,"[$-100A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Guatemala,8,"[$-100A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Bolivia,0,"dd/mm/yyyy;@")
FDATE(Bolivia,1,"dd/mm/yy;@")
FDATE(Bolivia,2,"d/m/yy;@")
FDATE(Bolivia,3,"dd\\-mm\\-yy;@")
FDATE(Bolivia,4,"yyyy\\-mm\\-dd;@")
FDATE(Bolivia,5,"[$-400A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Bolivia,6,"[$-400A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Bolivia,7,"[$-400A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Chile,0,"dd\\-mm\\-yyyy;@")
FDATE(Chile,1,"dd\\-mm\\-yy;@")
FDATE(Chile,2,"dd/mm/yy;@")
FDATE(Chile,3,"d/m/yy;@")
FDATE(Chile,4,"yyyy\\-mm\\-dd;@")
FDATE(Chile,5,"[$-340A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Chile,6,"[$-340A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Chile,7,"[$-340A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(DominicanRepublic,0,"dd/mm/yyyy;@")
FDATE(DominicanRepublic,1,"dd/mm/yy;@")
FDATE(DominicanRepublic,2,"mm/dd/yyyy;@")
FDATE(DominicanRepublic,3,"d/m/yy;@")
FDATE(DominicanRepublic,4,"dd\\-mm\\-yy;@")
FDATE(DominicanRepublic,5,"dd/mm/yyyy;@")
FDATE(DominicanRepublic,6,"[$-1C0A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(DominicanRepublic,7,"[$-1C0A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(DominicanRepublic,8,"[$-1C0A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Ecuador,0,"dd/mm/yyyy;@")
FDATE(Ecuador,1,"dd/mm/yy;@")
FDATE(Ecuador,2,"d/m/yy;@")
FDATE(Ecuador,3,"dd\\-mm\\-yy;@")
FDATE(Ecuador,4,"yyyy\\-mm\\-dd;@")
FDATE(Ecuador,5,"[$-300A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Ecuador,6,"[$-300A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Ecuador,7,"[$-300A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Mexico,0,"dd/mm/yyyy;@")
FDATE(Mexico,1,"dd/mm/yy;@")
FDATE(Mexico,2,"d/mm/yy;@")
FDATE(Mexico,3,"d/m/yy;@")
FDATE(Mexico,4,"dd\\-mm\\-yy;@")
FDATE(Mexico,5,"yyyy\\-mm\\-dd;@")
FDATE(Mexico,6,"[$-80A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Mexico,7,"[$-80A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Mexico,8,"[$-80A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Peru,0,"dd/mm/yyyy;@")
FDATE(Peru,1,"dd/mm/yy;@")
FDATE(Peru,2,"d/m/yy;@")
FDATE(Peru,3,"dd\\-mm\\-yy;@")
FDATE(Peru,4,"yyyy\\-mm\\-dd;@")
FDATE(Peru,5,"[$-280A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Peru,6,"[$-280A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Peru,7,"[$-280A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Paraguay,0,"dd/mm/yyyy;@")
FDATE(Paraguay,1,"dd/mm/yy;@")
FDATE(Paraguay,2,"d/m/yy;@")
FDATE(Paraguay,3,"dd\\-mm\\-yy;@")
FDATE(Paraguay,4,"yyyy\\-mm\\-dd;@")
FDATE(Paraguay,5,"[$-3C0A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Paraguay,6,"[$-3C0A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Paraguay,7,"[$-3C0A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(ElSalvador,0,"dd/mm/yyyy;@")
FDATE(ElSalvador,1,"dd/mm/yy;@")
FDATE(ElSalvador,2,"mm\\-dd\\-yyyy;@")
FDATE(ElSalvador,3,"yyyy\\-mm\\-dd;@")
FDATE(ElSalvador,4,"[$-440A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")

FDATE(TrinidadTobago,0,"dd/mm/yyyy;@")
FDATE(TrinidadTobago,1,"yyyy\\-mm\\-dd;@")
FDATE(TrinidadTobago,2,"[$-2C09]dddd\\,\\ dd\\ mmmm\\ yyyy;@")

FDATE(Honduras,0,"dd/mm/yyyy;@")
FDATE(Honduras,1,"dd/mm/yy;@")
FDATE(Honduras,2,"mm\\-dd\\-yyyy;@")
FDATE(Honduras,3,"yyyy\\-mm\\-dd;@")
FDATE(Honduras,4,"[$-480A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")

FDATE(Nicaragua,0,"dd/mm/yyyy;@")
FDATE(Nicaragua,1,"dd/mm/yy;@")
FDATE(Nicaragua,2,"mm\\-dd\\-yyyy;@")
FDATE(Nicaragua,3,"yyyy\\-mm\\-dd;@")
FDATE(Nicaragua,4,"[$-4C0A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")

FDATE(Venezuela,0,"dd/mm/yyyy;@")
FDATE(Venezuela,1,"dd/mm/yy;@")
FDATE(Venezuela,2,"d/m/yy;@")
FDATE(Venezuela,3,"dd\\-mm\\-yy;@")
FDATE(Venezuela,4,"yyyy\\-mm\\-dd;@")
FDATE(Venezuela,5,"[$-200A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Venezuela,6,"[$-200A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Venezuela,7,"[$-200A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Panama,0,"mm/dd/yyyy;@")
FDATE(Panama,1,"mm/dd/yy;@")
FDATE(Panama,2,"d/m/yy;@")
FDATE(Panama,3,"dd/mm/yy;@")
FDATE(Panama,4,"dd\\-mm\\-yy;@")
FDATE(Panama,5,"yyyy\\-mm\\-dd;@")
FDATE(Panama,6,"[$-180A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(Panama,7,"[$-180A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(Panama,8,"[$-180A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(CostaRica,0,"dd/mm/yyyy;@")
FDATE(CostaRica,1,"dd/mm/yy;@")
FDATE(CostaRica,2,"d/m/yy;@")
FDATE(CostaRica,3,"dd\\-mm\\-yy;@")
FDATE(CostaRica,4,"yyyy\\-mm\\-dd;@")
FDATE(CostaRica,5,"[$-140A]dddd\\,\\ dd\" de \"mmmm\" de \"yyyy;@")
FDATE(CostaRica,6,"[$-140A]dddd\\ d\" de \"mmmm\" de \"yyyy;@")
FDATE(CostaRica,7,"[$-140A]d\" de \"mmmm\" de \"yyyy;@")

FDATE(Jamaica,0,"dd/mm/yyyy;@")
FDATE(Jamaica,1,"yyyy\\-mm\\-dd;@")
FDATE(Jamaica,2,"[$-2009]dddd\\,\\ mmmm\\ dd\\,\\ yyyy;@")
FDATE(Jamaica,3,"[$-2009]mmmm\\ dd\\,\\ yyyy;@")
FDATE(Jamaica,4,"[$-2009]dddd\\,\\ dd\\ mmmm\\,\\ yyyy;@")
FDATE(Jamaica,5,"[$-2009]dd\\ mmmm\\,\\ yyyy;@")

FDATE(FrenchCanada,0,"yyyy\\-mm\\-dd;@")
FDATE(FrenchCanada,1,"yy\\-mm\\-dd;@")
FDATE(FrenchCanada,2,"dd\\-mm\\-yy;@")
FDATE(FrenchCanada,3,"yy\\ mm\\ dd;@")
FDATE(FrenchCanada,4,"dd/mm/yy;@")
FDATE(FrenchCanada,5,"[$-C0C]d\\ mmmm\\,\\ yyyy;@")
FDATE(FrenchCanada,6,"[$-C0C]d\\ mmm\\ yyyy;@")

FDATE(UnitedKingdom,0,"dd/mm/yyyy;@")
FDATE(UnitedKingdom,1,"dd/mm/yy;@")
FDATE(UnitedKingdom,2,"d/m/yy;@")
FDATE(UnitedKingdom,3,"d\\.m\\.yy;@")
FDATE(UnitedKingdom,4,"yyyy\\-mm\\-dd;@")
FDATE(UnitedKingdom,5,"[$-809]dd\\ mmmm\\ yyyy;@")
FDATE(UnitedKingdom,6,"[$-809]d\\ mmmm\\ yyyy;@")

FDATE(UnitedStates,0,"m/d;@")
FDATE(UnitedStates,1,"m/d/yy;@")
FDATE(UnitedStates,2,"mm/dd/yy;@")
FDATE(UnitedStates,3,"[$-409]d\\-mmm;@")
FDATE(UnitedStates,4,"[$-409]d\\-mmm\\-yy;@")
FDATE(UnitedStates,5,"[$-409]dd\\-mmm\\-yy;@")
FDATE(UnitedStates,6,"[$-409]mmm\\-yy;@")
FDATE(UnitedStates,7,"[$-409]mmmm\\-yy;@")
FDATE(UnitedStates,8,"[$-409]mmmm\\ d\\,\\ yyyy;@")
FDATE(UnitedStates,9,"[$-409]m/d/yy\\ h:mm\\ AM/PM;@")
FDATE(UnitedStates,10,"m/d/yy\\ h:mm;@")
FDATE(UnitedStates,11,"[$-409]mmmmm;@")
FDATE(UnitedStates,12,"[$-409]mmmmm\\-yy;@")
FDATE(UnitedStates,13,"m/d/yyyy;@")
FDATE(UnitedStates,14,"[$-409]d\\-mmm\\-yyyy;@")

FDATE(ISO,0,"yyyy\\-mm\\-dd;@")
FDATE(ISO,1,"yyyymmdd;@")
FDATE(ISO,2,"yy\\-mm\\-dd;@")
FDATE(ISO,3,"yymmdd;@")
FDATE(ISO,4,"yyyy\\-mm;@")
FDATE(ISO,5,"yyyy;@")
FDATE(ISO,6,"yyyy\\-mm\\-dd\\ hh:mm:ss;@")
FDATE(ISO,7,"yyyymmdd\"T\"hhmmss;@")

FDATE(Local,0,"dd/mm/yyyy hh:mm")  /* LOCALISED - VARIABLE */
FDATE(Local,1,"dd/mm/yyyy")        /* LOCALISED - VARIABLE */
FDATE(Local,2,"[$-F800]dddd\\,\\ mmmm\\ dd\\,\\ yyyy")

/* Time ; 27 countries with up to 8 formats each */
FTIME(Brazil,0,"h:mm;@")
FTIME(Brazil,1,"[$-409]h:mm\\ AM/PM;@")
FTIME(Brazil,2,"h:mm:ss;@")
FTIME(Brazil,3,"[$-409]h:mm:ss\\ AM/PM;@")
FTIME(Brazil,4,"mm:ss.0;@")
FTIME(Brazil,5,"[h]:mm:ss;@")
FTIME(Brazil,6,"[$-409]d/m/yy\\ h:mm\\ AM/PM;@")
FTIME(Brazil,7,"d/m/yy\\ h:mm;@")

FTIME(Uruguay,0,"[$-380A]hh:mm:ss\\ AM/PM;@")
FTIME(Uruguay,1,"[$-380A]h:mm:ss\\ AM/PM;@")
FTIME(Uruguay,2,"h:mm:ss;@")
FTIME(Uruguay,3,"hh:mm:ss;@")

FTIME(Argentina,0,"[$-2C0A]hh:mm:ss\\ AM/PM;@")
FTIME(Argentina,1,"[$-2C0A]h:mm:ss\\ AM/PM;@")
FTIME(Argentina,2,"hh:mm:ss;@")
FTIME(Argentina,3,"h:mm:ss;@")

FTIME(Colombia,0,"[$-240A]hh:mm:ss\\ AM/PM;@")
FTIME(Colombia,1,"[$-240A]h:mm:ss\\ AM/PM;@")
FTIME(Colombia,2,"h:mm:ss;@")
FTIME(Colombia,3,"hh:mm:ss;@")

FTIME(Guatemala,0,"[$-100A]hh:mm:ss\\ AM/PM;@")
FTIME(Guatemala,1,"[$-100A]h:mm:ss\\ AM/PM;@")
FTIME(Guatemala,2,"h:mm:ss;@")
FTIME(Guatemala,3,"hh:mm:ss;@")

FTIME(Bolivia,0,"[$-400A]hh:mm:ss\\ AM/PM;@")
FTIME(Bolivia,1,"[$-400A]h:mm:ss\\ AM/PM;@")
FTIME(Bolivia,2,"h:mm:ss;@")
FTIME(Bolivia,3,"hh:mm:ss;@")

FTIME(Chile,0,"h:mm:ss;@")
FTIME(Chile,1,"hh:mm:ss;@")

FTIME(DominicanRepublic,0,"[$-1C0A]hh:mm:ss\\ AM/PM;@")
FTIME(DominicanRepublic,1,"[$-1C0A]h:mm:ss\\ AM/PM;@")
FTIME(DominicanRepublic,2,"h:mm:ss;@")
FTIME(DominicanRepublic,3,"hh:mm:ss;@")

FTIME(Ecuador,0,"h:mm:ss;@")
FTIME(Ecuador,1,"hh:mm:ss;@")

FTIME(Mexico,0,"[$-80A]hh:mm:ss\\ AM/PM;@")
FTIME(Mexico,1,"[$-80A]h:mm:ss\\ AM/PM;@")
FTIME(Mexico,2,"h:mm:ss;@")
FTIME(Mexico,3,"hh:mm:ss;@")

FTIME(Peru,0,"[$-280A]hh:mm:ss\\ AM/PM;@")
FTIME(Peru,1,"[$-280A]h:mm:ss\\ AM/PM;@")
FTIME(Peru,2,"h:mm:ss;@")
FTIME(Peru,3,"hh:mm:ss;@")

FTIME(Paraguay,0,"[$-3C0A]hh:mm:ss\\ AM/PM;@")
FTIME(Paraguay,1,"[$-3C0A]h:mm:ss\\ AM/PM;@")
FTIME(Paraguay,2,"h:mm:ss;@")
FTIME(Paraguay,3,"hh:mm:ss;@")

FTIME(ElSalvador,0,"[$-440A]hh:mm:ss\\ AM/PM;@")
FTIME(ElSalvador,1,"hh:mm:ss;@")

FTIME(TrinidadTobago,0,"[$-409]hh:mm:ss\\ AM/PM;@")
FTIME(TrinidadTobago,1,"hh:mm:ss;@")

FTIME(Honduras,0,"[$-480A]hh:mm:ss\\ AM/PM;@")
FTIME(Honduras,1,"hh:mm:ss;@")

FTIME(Nicaragua,0,"[$-4C0A]hh:mm:ss\\ AM/PM;@")
FTIME(Nicaragua,1,"hh:mm:ss;@")

FTIME(Venezuela,0,"[$-200A]hh:mm:ss\\ AM/PM;@")
FTIME(Venezuela,1,"[$-200A]h:mm:ss\\ AM/PM;@")
FTIME(Venezuela,2,"h:mm:ss;@")
FTIME(Venezuela,3,"hh:mm:ss;@")

FTIME(Panama,0,"[$-180A]hh:mm:ss\\ AM/PM;@")
FTIME(Panama,1,"[$-180A]h:mm:ss\\ AM/PM;@")
FTIME(Panama,2,"h:mm:ss;@")
FTIME(Panama,3,"hh:mm:ss;@")

FTIME(CostaRica,0,"[$-140A]hh:mm:ss\\ AM/PM;@")
FTIME(CostaRica,1,"[$-140A]h:mm:ss\\ AM/PM;@")
FTIME(CostaRica,2,"h:mm:ss;@")
FTIME(CostaRica,3,"hh:mm:ss;@")

FTIME(Jamaica,0,"[$-409]hh:mm:ss\\ AM/PM;@")
FTIME(Jamaica,1,"[$-409]h:mm:ss\\ AM/PM;@")
FTIME(Jamaica,2,"h:mm:ss;@")
FTIME(Jamaica,3,"hh:mm:ss;@")

FTIME(FrenchCanada,0,"hh:mm:ss;@")
FTIME(FrenchCanada,1,"h:mm:ss;@")
FTIME(FrenchCanada,2,"h\" h \"mm;@")
FTIME(FrenchCanada,3,"h:mm;@")

FTIME(UnitedKingdom,0,"hh:mm:ss;@")
FTIME(UnitedKingdom,1,"h:mm:ss;@")
FTIME(UnitedKingdom,2,"[$-409]hh:mm:ss\\ AM/PM;@")
FTIME(UnitedKingdom,3,"[$-409]h:mm:ss\\ AM/PM;@")

FTIME(UnitedStates,0,"h:mm;@")
FTIME(UnitedStates,1,"[$-409]h:mm\\ AM/PM;@")
FTIME(UnitedStates,2,"h:mm:ss;@")
FTIME(UnitedStates,3,"[$-409]h:mm:ss\\ AM/PM;@")
FTIME(UnitedStates,4,"mm:ss.0;@")
FTIME(UnitedStates,5,"[h]:mm:ss;@")
FTIME(UnitedStates,6,"[$-409]m/d/yy\\ h:mm\\ AM/PM;@")
FTIME(UnitedStates,7,"m/d/yy\\ h:mm;@")

FTIME(ISO,0,"hh:mm:ss;@")
FTIME(ISO,1,"hhmmss;@")
FTIME(ISO,2,"hh:mm;@")
FTIME(ISO,3,"hhmm;@")
FTIME(ISO,4,"hh;@")
FTIME(ISO,5,"yyyy\\-mm\\-dd\\ hh:mm:ss;@")
FTIME(ISO,6,"yyyymmdd\"T\"hhmmss;@")

FTIME(Local,0,"[$-F400]h:mm:ss\\ AM/PM")
};

#define COUNTRY(x) { PCLASS(Country_##x), #x }

static const struct {
    PCLASS(Country)     country;
    const char         *name;
} countryNames[]={
    COUNTRY(Brazil),
    COUNTRY(Uruguay),
    COUNTRY(Argentina),
    COUNTRY(Colombia),
    COUNTRY(Guatemala),
    COUNTRY(Bolivia),
    COUNTRY(Chile),
    COUNTRY(DominicanRepublic),
    COUNTRY(Ecuador),
    COUNTRY(Mexico),
    COUNTRY(Peru),
    COUNTRY(Paraguay),
    COUNTRY(ElSalvador),
    COUNTRY(TrinidadTobago),
    COUNTRY(Honduras),
    COUNTRY(Nicaragua),
    COUNTRY(Venezuela),
    COUNTRY(Panama),
    COUNTRY(CostaRica),
    COUNTRY(Jamaica),
    COUNTRY(FrenchCanada),
    COUNTRY(UnitedKingdom),
    COUNTRY(UnitedStates),
    COUNTRY(ISO),
    COUNTRY(Local)
};

#define CURRENCY(x) { PCLASS(Currency_##x), #x }

static const struct
{
    PCLASS(Currency)    currency;
    const char         *name;
} currencyNames[]={
    CURRENCY(Brazil_Real),
    CURRENCY(Uruguay_Peso),
    CURRENCY(Argentina_Peso),
    CURRENCY(Colombia_Peso),
    CURRENCY(Guatemala_Quetzal),
    CURRENCY(Bolivia_Boliviano_Quechua),
    CURRENCY(Bolivia_Boliviano_Spanish),
    CURRENCY(Chile_Peso),
    CURRENCY(DominicanRepublic_Peso),
    CURRENCY(Ecuador_Sucre_Quechua),
    CURRENCY(Ecuador_Sucre_Spanish),
    CURRENCY(Mexico_Peso),
    CURRENCY(Peru_NuevoSol_Quechua),
    CURRENCY(Peru_NuevoSol_Spanish),
    CURRENCY(Paraguay_Guarani),
    CURRENCY(ElSalvador_Colon),
    CURRENCY(TrinidadTobago_Dollar),
    CURRENCY(Honduras_Lempira),
    CURRENCY(Nicaragua_Cordoba),
    CURRENCY(Venezuela_Bolivar),
    CURRENCY(Panama_Balboa),
    CURRENCY(CostaRica_Colon),
    CURRENCY(Jamaica_Dollar),
    CURRENCY(FrenchCanada_Dollar),
    CURRENCY(Sterling),
    CURRENCY(UnitedKingdom_Pound),
    CURRENCY(UnitedStates_Dollar),
    CURRENCY(Euro),
    CURRENCY(Euro_Alt),
    CURRENCY(Brazil_Real_Name),
    CURRENCY(Uruguay_Peso_Name),
    CURRENCY(Argentina_Peso_Name),
    CURRENCY(Colombia_Peso_Name),
    CURRENCY(Guatemala_Quetzal_Name),
    CURRENCY(Bolivia_Boliviano_Name),
    CURRENCY(Chile_Peso_Name),
    CURRENCY(DominicanRepublic_Peso_Name),
    CURRENCY(Ecuador_Sucre_Name),
    CURRENCY(Mexico_Peso_Name),
    CURRENCY(Peru_NuevoSol_Name),
    CURRENCY(Paraguay_Guarani_Name),
    CURRENCY(ElSalvador_Colon_Name),
    CURRENCY(TrinidadTobago_Dollar_Name),
    CURRENCY(Honduras_Lempira_Name),
    CURRENCY(Nicaragua_Cordoba_Name),
    CURRENCY(Venezuela_Bolivar_Name),
    CURRENCY(Panama_Balboa_Name),
    CURRENCY(CostaRica_Colon_Name),
    CURRENCY(Haiti_Gourde_Name),
    CURRENCY(Jamaica_Dollar_Name),
    CURRENCY(UnitedKingdom_Pound_Name),
    CURRENCY(UnitedStates_Dollar_Name),
    CURRENCY(Euro_Name)
};

/* How many subformats do we have for each category? */
static const int subformatsUsed[]={
    0, /* Category_General */
    0, /* Category_Text */
    3, /* Category_Number */
    1, /* Category_Fraction */
    1, /* Category_Percentage */
    1, /* Category_Scientific */
    3, /* Category_Currency */
    2, /* Category_Accounting */
    2, /* Category_Date */
    2, /* Category_Time */
    0, /* Category_Custom */
};

const char *FormatManager_countryName(unsigned int c)
{
    int i;

    for(i = 0; i < ARRAY_COUNT(countryNames); i++)
    {
        if (c == countryNames[i].country)
            return countryNames[i].name;
    }

    return NULL;
}

const char *FormatManager_currencyName(unsigned int c)
{
    int i;

    for(i = 0; i < ARRAY_COUNT(currencyNames); i++)
    {
        if (c == currencyNames[i].currency)
            return currencyNames[i].name;
    }

    return NULL;
}

unsigned long FormatManager_flagsFromString(const char *utf8Format)
{
    int i;

    for(i = 0; i < ARRAY_COUNT(formatBlock); i++)
    {
        if (0 == strcmp(utf8Format, formatBlock[i].format))
            return formatBlock[i].flags;
    }

    /* Not found, thus it's a custom format */
    return FormatManager_Category_Custom;
}

const char *FormatManager_stringFromPackedFlags(unsigned long flags)
{
    int i;

    for(i = 0; i < ARRAY_COUNT(formatBlock); i++)
    {
        if ( flags == formatBlock[i].flags )
            return formatBlock[i].format;
    }

    /* Not found - shouldn't happen */
    return NULL;
}

int FormatManager_countSubFormats(unsigned long match, int whichSub)
{
    int i;
    int count = 0;
    unsigned long mask = 0xff<<(whichSub*8); /* Always 8 bits per sub format */
    match &= ~mask;

    /* Run through our entire database, counting matches */
    for(i = 0; i < ARRAY_COUNT(formatBlock); i++)
    {
        if ( match == (formatBlock[i].flags &~ mask) )
            count++;
    }

    return count;
}

int FormatManager_subformatsForCategory(int category)
{
    if (category>=0 && category<ARRAY_COUNT(subformatsUsed) )
        return subformatsUsed[ category ];

    assert(!"Bad Category");
    return 0;
}
