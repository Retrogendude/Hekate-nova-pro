# Traducciones — versión 11
**Languages:** [Français](TRADUCTIONS.md) · [English](TRADUCTIONS.en.md) · **Español** · [Русский](TRADUCTIONS.ru.md)

El catálogo actual contiene 3922 claves exactas en inglés, francés, español, italiano, ruso, alemán y portugués. Los nuevos ajustes, textos de personalización y nombres legibles de los diagnósticos disponen de traducciones redactadas para los siete idiomas. Los nombres propios, identificadores técnicos y rutas se preservan. Las guías técnicas traducidas sin conexión y el texto de recuperación original conservan los límites de revisión descritos a continuación.

---
## Historial y procedencia de las traducciones

# Estado de las traducciones — Nova, versión 10

El catálogo tiene siete valores por clave: inglés, francés, español,
italiano, ruso, alemán y portugués. Los menús principales, ajustes,
tarjetas Herramientas/Información, confirmaciones listadas y la pantalla Apariencia
utilizan este catálogo. La prueba de RAM lanzada desde Nyx dispone de siete
paquetes completos para sus 108 cadenas de origen, incluidas sus comandos y errores.

El catálogo contiene 3 837 claves, con los siete idiomas para cada clave.
Las nuevas pantallas de Nova disponen de textos redactados en los siete idiomas;
sus rótulos en francés reproducen los de la maqueta. El archivo
`localization_nova.json` contiene estas adiciones y sus alias.
Las 1 268 cadenas de origen listadas en las 42 sondas nativas están
cubiertas, incluidos sus comentarios detallados. Los complementos de
informes se tradujeron sin conexión con Argos/CTranslate2 y después se
revisaron para preservar parámetros, números, rutas y notaciones técnicas.
201 entradas corregidas tienen prioridad sobre la traducción automática.
Los menús utilizan las traducciones redactadas para la interfaz.

La cobertura del catálogo no garantiza la calidad lingüística de
cada comentario automático. Los términos eléctricos críticos se
revisaron, pero una revisión de las descripciones largas por hablantes nativos
sigue siendo útil. La procedencia y las huellas de los modelos figuran en
`verification/provenance-traductions-techniques.json`. No se necesita ningún modelo
en la consola y en ella no se realiza ninguna traducción en línea.

Los nombres de componentes, registros, valores, unidades, rutas y nombres elegidos
por el usuario no se renombran. Los mensajes tempranos del cargador
y las visualizaciones de respaldo permanecen en inglés. El servicio de RAM
también usa el inglés cuando su paquete falta o no es válido. La traducción
de Nyx no cubre, por tanto, el conjunto de las pantallas tempranas de recuperación
del cargador, antes de la carga de la interfaz.

El inventario `verification/couverture-traductions.json` enumera las cadenas
restantes. También comprende los bytes de control de los botones, de los
identificadores internos, de los nombres de componentes y de los archivos: su
contador no es una tasa de traducción de la interfaz.
Los textos se conservan en UTF-8. Las comprobaciones verifican el orden de los
parámetros numéricos, los acentos, el cirílico, los iconos, los colores,
las duraciones de vida de los menús y los cortes de páginas multilingües.
