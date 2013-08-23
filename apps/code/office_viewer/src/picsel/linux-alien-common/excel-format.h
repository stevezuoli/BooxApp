/**
 * Excel Format Manager data and access interface
 *
 * This is a crude linear database and linear lookup for now
 * The declarations in this file DO NOT OBEY THE PCS LINE LENGTH rule,
 * for reasons of clarity of definition
 *
 * Copyright (C) Picsel 2009
 *
 * @author Picsel Technologies Ltd
 *
 * $Id: excel-format.h,v 1.2 2009/09/17 09:08:05 alans Exp $
 */

#ifndef EXCEL_FORMAT_H
#define EXCEL_FORMAT_H

#if defined(__cplusplus)
extern "C" {
#endif /* (__cplusplus) */

/* prepend "class" macro */
#define PCLASS(catname) FormatManager_##catname

#define ENABLE_LATAM_FORMATS

#if defined(ALIEN_ZH_TW) || defined(ALIEN_ZH_CN) || defined(ALIEN_BN_IN) || \
    defined(ALIEN_GU_IN) || defined(ALIEN_HI_IN) || defined(ALIEN_PA_IN) || \
    defined(ALIEN_TA_IN)
#undef ENABLE_LATAM_FORMATS
#endif /* ALIEN_ZH_TW || ALIEN_ZH_CN || ALIEN_BN_IN || ALIEN_GU_IN || ALIEN_HI_IN || ALIEN_PA_IN || ALIEN_TA_IN */

#define FORMAT_FLAGS(category,sub1,sub2,sub3 ) \
    ( category+(sub1<<8)+(sub2<<16)+(sub3<<24) )

#define FORMAT_CATEGORY_FROM_FLAGS(flags) ((flags) & 0xff)
#define FORMAT_SUB1_FROM_FLAGS(flags) (((flags)>> 8) & 0xff)
#define FORMAT_SUB2_FROM_FLAGS(flags) (((flags)>>16) & 0xff)
#define FORMAT_SUB3_FROM_FLAGS(flags) (((flags)>>24) & 0xff)

typedef enum Category
{
    PCLASS(Category_General) = 0,
    PCLASS(Category_Text),
    PCLASS(Category_Number),
    PCLASS(Category_Fraction),
    PCLASS(Category_Percentage),
    PCLASS(Category_Scientific),
    PCLASS(Category_Currency),
    PCLASS(Category_Accounting),
    PCLASS(Category_Date),
    PCLASS(Category_Time),
    PCLASS(Category_Custom)
} PCLASS(Category);

/* This enum may be reordered to put the users' favourite
   countries at the top.   For shared formatting rules,
   these would then be the "more likely to be matched" ones */
typedef enum Country
{
    PCLASS(Country_ISO) = 0,
    PCLASS(Country_UnitedKingdom),
    PCLASS(Country_UnitedStates),
    PCLASS(Country_FrenchCanada),
    PCLASS(Country_Local),
    PCLASS(Country_Brazil),
    PCLASS(Country_Uruguay),
    PCLASS(Country_Argentina),
    PCLASS(Country_Colombia),
    PCLASS(Country_Guatemala),
    PCLASS(Country_Bolivia),
    PCLASS(Country_Chile),
    PCLASS(Country_DominicanRepublic),
    PCLASS(Country_Ecuador),
    PCLASS(Country_Mexico),
    PCLASS(Country_Peru),
    PCLASS(Country_Paraguay),
    PCLASS(Country_ElSalvador),
    PCLASS(Country_TrinidadTobago),
    PCLASS(Country_Honduras),
    PCLASS(Country_Nicaragua),
    PCLASS(Country_Venezuela),
    PCLASS(Country_Panama),
    PCLASS(Country_CostaRica),
    PCLASS(Country_Jamaica),
    PCLASS(Country_Total) /* MUST be last enum */
} PCLASS(Country);

/* This enum may be reordered like the country enum */
typedef enum Currency
{
    PCLASS(Currency_UnitedKingdom_Pound) = 0,
    PCLASS(Currency_UnitedStates_Dollar),
    PCLASS(Currency_Euro),
    PCLASS(Currency_Euro_Alt),
    PCLASS(Currency_Sterling),
    PCLASS(Currency_FrenchCanada_Dollar),
    PCLASS(Currency_Brazil_Real),
    PCLASS(Currency_Uruguay_Peso),
    PCLASS(Currency_Argentina_Peso),
    PCLASS(Currency_Colombia_Peso),
    PCLASS(Currency_Guatemala_Quetzal),
    PCLASS(Currency_Bolivia_Boliviano_Quechua),
    PCLASS(Currency_Bolivia_Boliviano_Spanish),
    PCLASS(Currency_Chile_Peso),
    PCLASS(Currency_DominicanRepublic_Peso),
    PCLASS(Currency_Ecuador_Sucre_Quechua),
    PCLASS(Currency_Ecuador_Sucre_Spanish),
    PCLASS(Currency_Mexico_Peso),
    PCLASS(Currency_Peru_NuevoSol_Quechua),
    PCLASS(Currency_Peru_NuevoSol_Spanish),
    PCLASS(Currency_Paraguay_Guarani),
    PCLASS(Currency_ElSalvador_Colon),
    PCLASS(Currency_TrinidadTobago_Dollar),
    PCLASS(Currency_Honduras_Lempira),
    PCLASS(Currency_Nicaragua_Cordoba),
    PCLASS(Currency_Venezuela_Bolivar),
    PCLASS(Currency_Panama_Balboa),
    PCLASS(Currency_CostaRica_Colon),
    PCLASS(Currency_Jamaica_Dollar),
    PCLASS(Currency_UnitedKingdom_Pound_Name),
    PCLASS(Currency_UnitedStates_Dollar_Name),
    PCLASS(Currency_Euro_Name),
    PCLASS(Currency_Brazil_Real_Name),
    PCLASS(Currency_Uruguay_Peso_Name),
    PCLASS(Currency_Argentina_Peso_Name),
    PCLASS(Currency_Colombia_Peso_Name),
    PCLASS(Currency_Guatemala_Quetzal_Name),
    PCLASS(Currency_Bolivia_Boliviano_Name),
    PCLASS(Currency_Chile_Peso_Name),
    PCLASS(Currency_DominicanRepublic_Peso_Name),
    PCLASS(Currency_Ecuador_Sucre_Name),
    PCLASS(Currency_Mexico_Peso_Name),
    PCLASS(Currency_Peru_NuevoSol_Name),
    PCLASS(Currency_Paraguay_Guarani_Name),
    PCLASS(Currency_ElSalvador_Colon_Name),
    PCLASS(Currency_TrinidadTobago_Dollar_Name),
    PCLASS(Currency_Honduras_Lempira_Name),
    PCLASS(Currency_Nicaragua_Cordoba_Name),
    PCLASS(Currency_Venezuela_Bolivar_Name),
    PCLASS(Currency_Panama_Balboa_Name),
    PCLASS(Currency_CostaRica_Colon_Name),
    PCLASS(Currency_Haiti_Gourde_Name),
    PCLASS(Currency_Jamaica_Dollar_Name),
    PCLASS(Currency_Total) /* MUST be last enum */
} PCLASS(Currency);

const char *FormatManager_countryName(unsigned int c);
const char *FormatManager_currencyName(unsigned int c);
unsigned long FormatManager_flagsFromString(const char *utf8Format);
const char *FormatManager_stringFromPackedFlags(unsigned long flags);
int FormatManager_countSubFormats(unsigned long match, int whichSub);
int FormatManager_subformatsForCategory(int category);

#if defined(__cplusplus)
}
#endif /* (__cplusplus) */

#endif /* !EXCEL_FORMAT_H */
