
set(FONT_DROID_SANS_MONO_URL "https://googlefontdirectory.googlecode.com/hg/apache/droidsansmono/DroidSansMono.ttf")
set(FONT_DROID_SANS_MONO_FILE "DroidSansMono.ttf")
if (NOT EXISTS "${NARFBLOCK_DATA_PATH}/${FONT_DROID_SANS_MONO_FILE}")
	message(STATUS "Downloading ${FONT_DROID_SANS_MONO_FILE}")
	file(DOWNLOAD "${FONT_DROID_SANS_MONO_URL}" "${NARFBLOCK_DATA_PATH}/${FONT_DROID_SANS_MONO_FILE}")
endif()
