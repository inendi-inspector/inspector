REM
REM \file wintest.bat
REM
REM Copyright (C) Picviz Labs 2010-2012

SET PICVIZ_NORMALIZE_HELPERS_DIR=..\..\..\libpicviz\plugins\normalize-helpers\
SET PICVIZ_NORMALIZE_DIR=..\..\..\libpicviz\plugins\normalize\RelWithDebInfo\
SET PICVIZ_DEBUG_LEVEL=DEBUG
SET PICVIZ_DEBUG_FILE=debug.txt

PATH=%PATH%;D:\cactuslabs\trunk\libpicviz\src\RelWithDebInfo

picviz-inspector.exe


