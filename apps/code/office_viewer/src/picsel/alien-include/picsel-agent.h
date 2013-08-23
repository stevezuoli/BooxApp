/**
 * Functions for initialising the document agents
 *
 * The SDK supplied to the integrator will only contain the entry points
 * for the agents included in the customer's contract.
 *
 * @file
 * $Id: picsel-agent.h,v 1.22 2008/12/11 16:34:20 roger Exp $
 *
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvInitialiseAgents Document Agent Initialisation
 * @ingroup TgvInitialisation
 *
 * Initialisation functions for the document agents, each of which
 * can handle a given file format.
 *
 * These functions may only be called from AlienConfig_initialiseAgents().
 * Calling each of these functions will increase the size of the linked
 * executable: see @ref TgvLink_Time_Configuration.
 *
 * The SDK supplied will only contain the entry points for the agents
 * specified in the customer's contract.
 * @{
 */

#ifndef PICSEL_AGENT_H
#define PICSEL_AGENT_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Make the HTML document agent available for use by the Alien application.
 * This agent is used for parsing content of the HTML file and converting
 * it into Picsel's internal format for rendering the document onto
 * the Device's screen. HTML is normally used on web pages, and sometimes
 * in emails.
 *
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported HTML features. Please
 * contact your Picsel support representative regarding the specification.
 *
 * By convention, HTML data format files use a file extension .htm or .html
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be HTML.
 *
 * @param picselContext  Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Html(Picsel_Context *picselContext);

/**
 * Make the Microsoft Word document agent available for use by the Alien
 * application. This agent is used for parsing the MS Word file and
 * converting its content into Picsel's internal format for rendering
 * the document onto the Device's screen.
 *
 * MS Word is an application designed primarily for word processing.
 * MS Word files can also contain graphics on pages, tables and other
 * elements. This format is widely used. The MS Word agent supports
 * versions MS Word 97 - 2004. Please refer to the Document Format
 * Support Specification (PICSEL-ESF-0023) for a full list of supported
 * versions and the application features. Please contact your Picsel support
 * representative regarding the specification.
 *
 * By convention, the MS Word data format files use a file extension .doc
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @note For MS Word 2007 (Office Open XML) version of the MS Word file
 * format please see @ref Picsel_Agent_Ooxml().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be MS Word.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Word(Picsel_Context *picselContext);

/**
 * Make the Microsoft Excel document agent available for use by the Alien
 * application. This agent is used for parsing the MS Excel file and
 * converting its content into Picsel's internal format for rendering
 * the document onto the Device's screen.
 *
 * MS Excel is a tool to create and format spreadsheets, analyse
 * information, represent numeric data in a graphic form like charts,
 * diagrams.
 *
 * The MS Excel files can also include other graphic elements on pages.
 *
 * The MS Excel agent supports versions MS Excel 95 - 2003. Please refer
 * to the Document Format Support Specification (PICSEL-ESF-0023) for
 * a full list of supported versions and the application features.
 * Please contact your Picsel support representative regarding
 * the specification.
 *
 * By convention, the MS Excel data format files use a file extension .xls
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @note For MS Excel 2007 (Office Open XML) version of the MS Excel file
 * format please see @ref Picsel_Agent_Ooxml().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be MS Excel.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Excel(Picsel_Context *picselContext);

/**
 * Make the Microsoft PowerPoint document agent available for use by
 * the Alien application. This agent is used for parsing the MS PowerPoint
 * file and converting its content into Picsel's internal format for
 * rendering the document onto the Device's screen.
 *
 * The MS PowerPoint is a presentation tool as well as a simple multimedia
 * development tool that enables users to create high-impact, dynamic
 * presentations. Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported versions and
 * the application features. Please contact your Picsel support
 * representative regarding the specification.
 *
 * By convention, the MS PowerPoint data format files use a file
 * extension .ppt but the Picsel library indentifies file types based on
 * their contents rather than their file extension.
 *
 * @note For MS PowerPoint 2007 (Office Open XML) version of
 * the MS PowerPoint file format please see @ref Picsel_Agent_Ooxml().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be
 * MS PowerPoint.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_PowerPoint(Picsel_Context *picselContext);

/**
 * Make the ePage File Information Format (EFIF) agent available for
 * use by the Alien application. This agent is used for parsing
 * the EFIF file and converting its content into Picsel's internal
 * format for rendering the document onto the Device's screen.
 * The format has been designed by Picsel to be as small as possible
 * and to be a good target for compression. At the moment, the EFIF
 * data format file can be created by Picsel servers only.
 *
 * By convention, the EFIF data format files use a file extension .efif
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be EFIF.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Efif(Picsel_Context *picselContext);

/**
 * Make the Image agent available for use by the Alien application.
 * This agent is used for parsing graphic file formats and converting
 * them into Picsel's internal format for rendering images onto
 * the Device's screen.
 *
 * The image component supports JPEG, BMP, GIF, PNG and other graphic
 * formats. Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported graphic formats.
 * Please contact your Picsel support representative regarding
 * the specification.
 *
 * The file extension of picture files depends on the format. Popular
 * extensions are .jpg, .jpeg, .gif, .png, .bmp. The Picsel library
 * indentifies file types based on their contents rather than their
 * file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be
 * a supported image format.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Image(Picsel_Context *picselContext);

/**
 * Make the Text agent available for use by the Alien application.
 * This agent is used for converting plain text files into
 * Picsel's internal format for rendering the document
 * onto the Device's screen.
 *
 * By convention, plain text files use a file extension .txt
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be
 * a plain text.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Text(Picsel_Context *picselContext);

/**
 * Make the Portable Document Format (PDF) document agent available for
 * use by the Alien application. This agent is used for parsing the PDF
 * file and converting its content into Picsel's internal format for
 * rendering the document onto the Device's screen.
 *
 * The PDF is a file format developed by Adobe to deliver and render
 * electronic documents created for print. It preserves the exact look
 * of the original document.
 *
 * By convention, the PDF data format files use a file extension .pdf
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported PDF features. Please
 * contact your Picsel support representative regarding the specification.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be PDF.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Pdf(Picsel_Context *picselContext);

/**
 * Make the Microsoft Windows Meta File (WMF) agent available for use by
 * the Alien application. This agent is used for parsing the WMF file and
 * converting its content into Picsel's internal format for rendering
 * the document onto the Device's screen.
 *
 * The WMF is a graphics file format designed by Microsoft and is intendend
 * be portable between applications and may contain both vector and bitmap
 * parts. In contrast to raster formats (.JPG, .GIF), WMF is generally used
 * to store line art or drawings.
 *
 * By convention, the WMF data format files use a file extension .wmf
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be WMF.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Wmf(Picsel_Context *picselContext);

/**
 * Make the MIME HTML (MHTML) document agent available for use by the Alien
 * application. This agent is used for parsing the MHTML file
 * and converting its content into Picsel's internal format
 * for rendering the document onto the Device's screen.
 *
 * The MHTML is used to bind resources represented as external links,
 * such as images, together with HTML code into a single file. This
 * format allows users to save a web page and its resources as
 * a single MHTML fle.
 *
 * By convention, the MHTML data format files use a file extension .mht
 * or .mhtml but the Picsel library indentifies file types based on their
 * contents rather than their file extension.
 *
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported MHTML features. Please
 * contact your Picsel support representative regarding the specification.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be MHTML.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Mhtml(Picsel_Context *picselContext);

/**
 * Make the Audio agent available for use by the Alien application.
 * This agent is used for converting audio data and playing it on
 * the Device. The Picsel library supports several audio formats.
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported audio formats.
 * Please contact your Picsel support representative regarding
 * the specification.
 *
 * The file extension depends on the type of audio format. The Picsel
 * library indentifies file types based on their contents rather
 * than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be
 * a supported audio format.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Audio(Picsel_Context *picselContext);

/**
 * Make the Adobe Flash document agent available for use by the Alien
 * application. This agent is used for parsing the Flash data file and
 * converting its content into Picsel's internal format for rendering
 * the document onto the Device's screen.
 *
 * Adobe Flash data format is popular for creating rich Internet
 * applications, increasing interactivity of web pages and animation.
 *
 * By convention, the Adobe Flash data format files use a file
 * extension .swf but the Picsel library indentifies file types based
 * on their contents rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be Flash.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Flash(Picsel_Context *picselContext);

/**
 * Make the Comma Separated Values (CSV) document agent available for use by
 * the Alien application. This agent is used for parsing the CSV file
 * and converting its content into Picsel's internal format
 * for rendering the document onto the Device's screen.
 *
 * The CSV is a delimited format that has columns separated by
 * the comma character and is used to store tabular data. The CSV format
 * is simple and supported by almost all spreadsheets and database management
 * systems.
 *
 * By convention, the CSV data format files use a file extension .csv or .txt
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported CSV features. Please
 * contact your Picsel support representative regarding the specification.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be CSV.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Csv(Picsel_Context *picselContext);

/**
 * Make the Hangul document agent available for use by the Alien
 * application. This agent is used for parsing the Hangul file and
 * converting its content into Picsel's internal format for rendering
 * the document onto the Device's screen.
 *
 * Hangul is a proprietary word processing application designed
 * to support the particular needs of the Korean written language.
 * Please refer to the Document Format Support Specification (PICSEL-ESF-0023)
 * for a full list of supported versions of the Hangul word processor
 * and the application features.  Please contact your Picsel support
 * representative regarding the specification.
 *
 * By convention, the Hangul data format files use a file extension .hwp
 * but the Picsel library indentifies file types based on their contents
 * rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be
 * a Hangul document.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Hangul(Picsel_Context *picselContext);

/**
 * Make the Office Open XML document agent available for use by
 * the Alien application. This agent is used for parsing the OOXML
 * file and converting its content into Picsel's internal format for
 * rendering the document onto the Device's screen.
 *
 * Office Open XML is a file format for representing spreadsheets,
 * charts, presentations and word processing documents. Office Open XML
 * document may contain several documents encoded into specialised markup
 * languages corresponding to applications within the MS Office product
 * line.
 *
 * Please refer to the Document Format Support Specification (PICSEL-ESF-0023)
 * for a full list of supported versions and the application features.  Please
 * contact your Picsel support representative regarding the specification.
 *
 * By convention, the OOXML data format files use a file extension .pptx,
 * .xlsx, and .docx but the Picsel library indentifies file types based on
 * their contents rather than their file extension.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @note Even if the agent has been initialised, it will not be used
 * unless the Picsel library finds a file that it considers to be OOXML.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Ooxml(Picsel_Context *picselContext);

/**
 * Make the Browser Plugin agent available for use by the Alien application.
 *
 * The Browser Plugin enables the Alien application to use
 * pre-installed plugins for managing content not supported by
 * the Picsel library. The Browser Plugin agent will embed
 * this content into the document which will be displayed on
 * the Device screen by the Picsel library.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_BrowserPlugin(Picsel_Context *picselContext);

/**
 * Make the Deco-mail document agent available for use by the Alien
 * application. This agent is used for parsing the Deco-mail file and
 * converting its content into Picsel's internal format for rendering
 * the document onto the Device's screen.
 *
 * Deco-mail is a service that allows users to change background or
 * font colour of i-mode emails, attaching images and animations to them.
 * Deco-mails are written in standard HTML and can be viewed on
 * a regular PC.
 *
 * By convention, the Deco-mail data format files use a file
 * extension .html or .htm but the Picsel library indentifies file types
 * based on their contents rather than their file extension.
 *
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported Deco-mail features.
 * Please contact your Picsel support representative regarding
 * the specification.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_Decomail(Picsel_Context *picselContext);

/* ---------------------------------------------------------------------- */
/* For Picsel use only                                                    */
/* ---------------------------------------------------------------------- */
/**
 * Make all document agents in the Picsel library available for use
 * by the Alien application.
 *
 * Please refer to the Document Format Support Specification
 * (PICSEL-ESF-0023) for a full list of supported features by
 * all document agents.
 *
 * @warning This is an internal API and not suitable for use by the Alien
 * application integrator.
 *
 * @pre This call should only be made from inside of
 * AlienConfig_initialiseAgents().
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 *
 * @retval 0 if an error occurred,
 * @retval 1 success
 */
int Picsel_Agent_RegisterAll(Picsel_Context *picselContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_AGENT_H */
