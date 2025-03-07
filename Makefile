# mysql_fdw/Makefile
#
# Portions Copyright (c) 2012-2014, PostgreSQL Global Development Group
# Portions Copyright (c) 2004-2024, EnterpriseDB Corporation.
#

MODULE_big = mysql_fdw
OBJS = connection.o option.o deparse.o mysql_query.o mysql_fdw.o mysql_pushability.o postgres_parquet.o parquet_writer.o


EXTENSION = mysql_fdw
DATA = mysql_fdw--1.0.sql mysql_fdw--1.1.sql mysql_fdw--1.0--1.1.sql mysql_fdw--1.2.sql mysql_fdw--1.1--1.2.sql mysql_fdw--1.3.sql mysql_fdw--1.2--1.3.sql mysql_fdw_pushdown.config

REGRESS = server_options connection_validation dml select pushdown join_pushdown aggregate_pushdown limit_offset_pushdown misc

MYSQL_CONFIG = mysql_config
PG_CPPFLAGS := $(shell $(MYSQL_CONFIG) --include)
LIB := $(shell $(MYSQL_CONFIG) --libs)

# In Debian based distros, libmariadbclient-dev provides mariadbclient (rather than mysqlclient)
ifneq ($(findstring mariadbclient,$(LIB)),)
MYSQL_LIB = mariadbclient
else
MYSQL_LIB = mysqlclient
endif

UNAME = uname
OS := $(shell $(UNAME))
ifeq ($(OS), Darwin)
DLSUFFIX = .dylib
else
DLSUFFIX = .so
endif

PG_CPPFLAGS += -D _MYSQL_LIBNAME=\"lib$(MYSQL_LIB)$(DLSUFFIX)\"

PG_CPPFLAGS += -I$(HOME)/include/arrow
PG_CPPFLAGS += -I$(HOME)/include/parquet 
PG_CPPFLAGS += -I$(HOME)/include/parquet/arrow
SHLIB_LINKS = -L$(HOME)/lib -larrow -lparquet -larrow_dataset -lstdc++
CXX = g++
CXXFLAGS = -std=c++13 -Wall -Werror
CPPFLAGS =  -I$(HOME)/include 
LDFLAGS += -L$(HOME)/lib -larrow -lparquet -lstdc++

SHLIB_LINK += -L/home/killi-pt7716/lib -larrow -lparquet
PG_CXXFLAGS += -std=c++17
SHLIB_LINK += -lstdc++

# Add Arrow and Parquet include and library directories
ARROW_CFLAGS = $(shell pkg-config --cflags arrow)
ARROW_LIBS = $(shell pkg-config --libs arrow)
PARQUET_CFLAGS = $(shell pkg-config --cflags parquet)
PARQUET_LIBS = $(shell pkg-config --libs parquet)

PG_CPPFLAGS += $(ARROW_CFLAGS) $(PARQUET_CFLAGS)
SHLIB_LINK += $(ARROW_LIBS) $(PARQUET_LIBS)

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
ifndef MAJORVERSION
MAJORVERSION := $(basename $(VERSION))
endif
ifeq (,$(findstring $(MAJORVERSION), 12 13 14 15 16 17))
$(error PostgreSQL 12, 13, 14, 15, 16, or 17 is required to compile this extension)
endif

else
subdir = contrib/mysql_fdw
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

