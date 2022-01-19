LIBRARY_NAME = rcp
# PD or MAX
EXT_TARGET = PD
NO_SSL = false

# rcp external
RCP_LIB_EXT = $(shell find sources -name *.cpp)

# libraries
DEPENDENCIES_BASE = dependencies

# rcp-c
RCP_SRC = $(shell find $(DEPENDENCIES_BASE)/rcp-c -name *.c)
RCP_INCLUDE = $(DEPENDENCIES_BASE)/rcp-c

# 3rd party
FLEXT_INCLUDE = $(DEPENDENCIES_BASE)/flext/source
ASIO_INCLUDE = $(DEPENDENCIES_BASE)/asio/asio/include
WEBSOCKETPP_INCLUDE = $(DEPENDENCIES_BASE)/websocketpp

# openssl
OPENSSL_BASE = ../openssl-1.1.1m
COPY_OPENSSL = true

# max/msp
CYCLING_BASE = ../max-sdk-8.0.3/source/c74support
MAX_INCLUDE = $(CYCLING_BASE)/max-includes
MSP_INCLUDE = $(CYCLING_BASE)/msp-includes


#$(info $$var is [${OS}])

ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
    REMOVE_DEAD =
    FUNCTION_SECTIONS=
    FLEXT_CPPFLAGS=
    ADDITIONAL_cflags=-D__USE_W32_SOCKETS -D_WIN32_WINDOWS=0x0601
    MINGW_ldflags=-Wl,--enable-auto-import $(PD_BIN)/pd.dll -Lwin32/lib
    POST_ldflags= -lflext-pd_s.0.6.0
    MACOS_VERSION_MIN=
    OPENSSL_INCLUDE=/usr/include
    OPENSSL_ldflags=
    ADDITIONAL_ldflags=
else
    UNAME_S := $(shell uname -s)
    MINGW_ldflags=
    POST_ldflags =
    ADDITIONAL_cflags=
    FLEXT_CPPFLAGS=-DFLEXT_INLINE
    FUNCTION_SECTIONS=-fdata-sections -ffunction-sections
    ifeq ($(UNAME_S),Linux)
        REMOVE_DEAD = -Wl,--gc-sections
        MACOS_VERSION_MIN=
        OPENSSL_INCLUDE = $(OPENSSL_BASE)/include
        OPENSSL_ldflags = -L$(OPENSSL_BASE) -lssl -lcrypto
    endif
    ifeq ($(UNAME_S),Darwin)
        
        REMOVE_DEAD = -dead_strip

        #
        #ADDITIONAL_cflags += -isysroot /path/to/MacOSX10.13.sdk
        # minimum 10.9 for libc++
        ADDITIONAL_cflags += -mmacosx-version-min=10.9
        
        ifeq ($(EXT_TARGET),MAX)
            # attention: this needs multi-arch openssl
            ADDITIONAL_cflags += -arch x86_64
            ADDITIONAL_cflags += -arch i386
        endif
        
        # ssl installed via e.g. brew
        #OPENSSL_INCLUDE = /usr/local/opt/openssl@1.1/include
        #OPENSSL_ldflags = -L/usr/local/opt/openssl@1.1/lib -lssl -lcrypto
        
        # ssl in OPENSSL_BASE
        OPENSSL_INCLUDE = $(OPENSSL_BASE)/include
        OPENSSL_ldflags = -L$(OPENSSL_BASE) -lssl -lcrypto
    endif
endif

FLEXT_CPPFLAGS += -DFLEXT_ATTRIBUTES=1
ADDITIONAL_cflags += -DLIBRARY_NAME=$(LIBRARY_NAME)

CPPFLAGS = $(FLEXT_CPPFLAGS)
cflags = -I$(RCP_INCLUDE) -I$(FLEXT_INCLUDE) -I$(ASIO_INCLUDE) -I$(WEBSOCKETPP_INCLUDE) $(ADDITIONAL_cflags)

# use c++11 for STL in ASIO and websocketpp
CXXFLAGS = -std=c++11

ldflags = $(REMOVE_DEAD) $(MINGW_ldflags) $(ADDITIONAL_ldflags)

ifeq ($(NO_SSL),false)
    cflags += -I$(OPENSSL_INCLUDE)
    ldflags += $(OPENSSL_ldflags)
else
    # turn off ssl
    cflags += -DRCP_PD_NO_SSL
endif


#################################
# PD
#################################
ifeq ($(EXT_TARGET),PD)
    
    CPPFLAGS += -DPD
    
    # using lib-pd-builder
    # figure out compilation targets etc
    
    
    # lib-pd-builder
    lib.name = $(LIBRARY_NAME)
    # NOTE: usign a modified version of Makefile.pflibbuilder
    #       introducing: depcheck.ignore to ignore dependency check
    # ignore depcheck (c++11 not defined in depcheck, so asio fails)
    depcheck.ignore = true
    
    suppress-wunused = yes
    cxx.flags = $(CXXFLAGS)
    common.sources = $(RCP_SRC)
    # compile as lib
    lib.setup.sources= $(RCP_LIB_EXT)
    make-lib-executable = yes
    
    PDINCLUDEDIR = $(DEPENDENCIES_BASE)/pd
    
    # include pd-lib-builder
    include pd-lib-builder/Makefile.pdlibbuilder
    

ifeq ($(UNAME_S),Windows)
    # TODO: windows
endif

ifeq ($(UNAME_S),Linux)
    # linux - nothing to do?
endif

ifeq ($(UNAME_S),Darwin)

post:
	cp $(OPENSSL_BASE)/libcrypto.1.1.dylib ./
	cp $(OPENSSL_BASE)/libssl.1.1.dylib ./
	install_name_tool -id @loader_path/libcrypto.1.1.dylib libcrypto.1.1.dylib
	install_name_tool -id @loader_path/libssl.1.1.dylib libssl.1.1.dylib
	install_name_tool -change /usr/local/lib/libcrypto.1.1.dylib @loader_path/libcrypto.1.1.dylib libssl.1.1.dylib
	
	file $(LIBRARY_NAME).pd_darwin
	install_name_tool -change /usr/local/lib/libcrypto.1.1.dylib @loader_path/libcrypto.1.1.dylib $(LIBRARY_NAME).pd_darwin
	install_name_tool -change /usr/local/lib/libssl.1.1.dylib @loader_path/libssl.1.1.dylib $(LIBRARY_NAME).pd_darwin
endif
endif


#################################
# Max
#################################
ifeq ($(EXT_TARGET),MAX)

    cflags += -DMAXMSP
    
    ODIR=obj

    cflags += -I$(MAX_INCLUDE) -I$(MSP_INCLUDE) -F$(MAX_INCLUDE) -F$(MSP_INCLUDE)
    ldflags += -framework MaxAPI -framework MaxAudioAPI
   
    OBJ = $(patsubst %,$(ODIR)/%, $(RCP_SRC:.c=.o) $(RCP_LIB_EXT:.cpp=.o))   
    #$(info OBJ = ${OBJ})
    
    
$(ODIR)/%.o: %.c $(RCP_SRC)
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(cflags)
	
$(ODIR)/%.o: %.cpp $(RCP_LIB_EXT)
	mkdir -p $(dir $@)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(CPPFLAGS) $(cflags)
	
.PHONY: clean all post

ifeq ($(UNAME_S),Windows)
    # TODO: windows
endif

ifeq ($(UNAME_S),Darwin)
    # darwin
    
all: $(OBJ)
	$(CXX) -shared -fPIC -o $(LIBRARY_NAME) $^ $(CXXFLAGS) $(cflags) $(ldflags)

	# post process
	install_name_tool -change /usr/local/lib/libcrypto.1.1.dylib @loader_path/libcrypto.1.1.dylib $(LIBRARY_NAME)
	install_name_tool -change /usr/local/lib/libssl.1.1.dylib @loader_path/libssl.1.1.dylib $(LIBRARY_NAME)
	
	rm -rf $(LIBRARY_NAME).mxo
	cp -r package/template.mxo $(LIBRARY_NAME).mxo
	mv $(LIBRARY_NAME) $(LIBRARY_NAME).mxo/Contents/MacOS/
	
	# copy libs
	cp $(OPENSSL_BASE)/libcrypto.1.1.dylib $(LIBRARY_NAME).mxo/Contents/MacOS/
	cp $(OPENSSL_BASE)/libssl.1.1.dylib $(LIBRARY_NAME).mxo/Contents/MacOS/
	
	install_name_tool -id @loader_path/libcrypto.1.1.dylib $(LIBRARY_NAME).mxo/Contents/MacOS/libcrypto.1.1.dylib
	install_name_tool -id @loader_path/libssl.1.1.dylib $(LIBRARY_NAME).mxo/Contents/MacOS/libssl.1.1.dylib
	install_name_tool -change /usr/local/lib/libcrypto.1.1.dylib @loader_path/libcrypto.1.1.dylib $(LIBRARY_NAME).mxo/Contents/MacOS/libssl.1.1.dylib
endif
	

clean:
	rm -rf $(ODIR)
	rm -rf $(LIBRARY_NAME) $(LIBRARY_NAME).mxo
	
endif
