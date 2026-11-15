# Translations — version 11
**Languages:** [Français](TRADUCTIONS.md) · **English** · [Español](TRADUCTIONS.es.md) · [Русский](TRADUCTIONS.ru.md)

The current catalogue contains 3922 exact keys in English, French, Spanish, Italian, Russian, German and Portuguese. The new settings, customisation texts and human-readable diagnostic names have translations written for all seven languages. Proper names, technical identifiers and paths are preserved. The technical guides translated offline and the original recovery text keep the revision limits described below.

---
## History and provenance of the translations

# Translation status — Nova, version 10

The catalogue has seven values per key: English, French, Spanish,
Italian, Russian, German and Portuguese. The main menus, settings,
Tools/Information cards, listed confirmations and the Appearance screen
use this catalogue. The RAM test launched from Nyx has seven complete
packs for its 108 source strings, including its commands and errors.

The catalogue contains 3 837 keys, with all seven languages for each key.
The new Nova screens have texts written in all seven languages;
their French labels reproduce those of the mock-up. The file
`localization_nova.json` contains these additions and their aliases.
The 1 268 source strings listed in the 42 native probes are
covered, including their detailed comments. The report supplements
were translated offline with Argos/CTranslate2, then checked
to preserve parameters, numbers, paths and technical notation.
201 corrected entries take priority over the machine translation.
The menus use the translations written for the interface.

Catalogue coverage does not guarantee the language quality of
each automatic comment. The critical electrical terms were
reviewed, but a review of the long descriptions by native speakers
remains useful. The provenance and fingerprints of the models are in
`verification/provenance-traductions-techniques.json`. No model is
required on the console and no online translation is performed there.

Component names, registers, values, units, paths and names chosen
by the user are not renamed. The loader's early messages
and the fallback displays remain in English. The RAM service also
uses English when its pack is missing or invalid. The translation
of Nyx therefore does not cover all the loader's early recovery
screens, before the interface is loaded.

The inventory `verification/couverture-traductions.json` lists the
remaining strings. It also includes the control bytes of the buttons,
of the internal identifiers, of the component names and of the files: its
count is not a translation rate of the interface.
The texts are kept in UTF-8. The checks verify the order of numeric
parameters, accents, Cyrillic, icons, colours,
menu lifetimes and multilingual page breaks.
