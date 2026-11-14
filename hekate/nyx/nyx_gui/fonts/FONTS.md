# Polices depuis la carte SD

Le menu Apparence propose une police et une taille principale de 20 à 36 pixels.
Après « Enregistrer et recharger », la police choisie s'applique
aux menus. Les petits libellés suivent la même famille avec une taille adaptée.
Les rapports conservent une police à chasse fixe pour aligner les colonnes.

Trois fichiers prêts à l'emploi sont fournis dans `fonts/` à la racine du paquet :

- `Lecture.nyxf` : Noto Sans régulier.
- `Lecture-renforcee.nyxf` : trait renforcé pour le contraste.
- `Lecture-fixe.nyxf` : largeur de caractères fixe.

Pour ajouter une autre police, convertir son fichier TTF ou OTF sur ordinateur,
puis copier le résultat `.nyxf` dans le dossier `fonts` à la racine de la SD.
Sous Windows, le paquet propose aussi `outils-polices/convertir-police.cmd` :
glisser le fichier TTF/OTF sur ce programme crée le `.nyxf` à côté du fichier
original. Conserver ensemble les dossiers `tools/` et `source/` fournis.
Les TTF/OTF bruts ne sont pas chargés directement par la console. Ce format
bitmap permet de vérifier les tailles, les glyphes et les limites de lecture
avant de les utiliser ; il ne lance pas de programme fourni par une police.

```text
python -m pip install Pillow
python convert_font.py MaPolice.ttf -o MaPolice.nyxf
```

Le convertisseur se trouve dans les sources sous
`nyx/nyx_gui/fonts/tools/convert_font.py`. Conserver ce dossier avec les dossiers
`source/` et le générateur Unicode voisin : Noto Sans sert de secours pour les
caractères manquants. Utiliser uniquement des polices dont la licence autorise
la conversion ; fournir leur licence lors d'une redistribution.

Le dossier est relu au prochain chargement de Nyx. Jusqu'à 16 fichiers `.nyxf`
supplémentaires sont proposés, classés par nom. Un nom doit être court (moins
de 64 caractères), composé de lettres ASCII, chiffres, espaces, `_`, `-` et `.`,
sans dossier ni `..` et avec extension `.nyxf` (majuscules acceptées).
Un fichier absent, invalide ou incomplet utilise la police
intégrée. Les réglages de langue et de police sont indépendants.

Les variantes fournies dérivent de Noto Sans sous SIL OFL 1.1. La police source
et le texte de licence sont inclus dans `source/`. Le convertisseur conserve
ASCII, les alphabets latins étendus, le cyrillique et la ponctuation utilisés
par les sept langues de Nyx. Il produit toutes les tailles prises en charge.

## Format NYXF 1

Entiers little endian. En-tête 32 octets : `NYXF`, version u16=1, taille u16=32,
taille totale u32, nombre de sections u32 (1..24), table offset u32=32,
réservé u32=0, FNV-1a u32 de la table, FNV-1a u32 des 28 premiers octets.

Chaque section a 32 octets : hauteur, nombre de glyphes, offset de la table
de glyphes, offset des bitmaps, longueur des bitmaps, fin de section,
FNV-1a de la section et réservé=0 (u32). Une entrée de glyphe contient
codepoint, largeur et offset relatif du bitmap (trois u32). Les glyphes sont
strictement triés, les données contiguës. Couverture minimale ASCII et
cyrillique ; 1024 glyphes au maximum, largeur 1..128, bitmaps 4 bpp avec chaque
ligne arrondie à l'octet. Taille du fichier 16 Mio maximum ; section 2 Mio.
Les deux sections correspondant aux tailles régulière et petite sont lues.
