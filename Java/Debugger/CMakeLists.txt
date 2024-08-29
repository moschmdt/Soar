cmake_minimum_required(VERSION 3.22)
project(SoarDebugger VERSION 9.6.3)

file(GLOB_RECURSE JAVA_SOURCES
        "src/*.java")

message(${JAVA_SOURCES})

find_package(Java)

add_jar(Debugger
        SOURCES ${JAVA_SOURCES}
        # [RESOURCES NAMESPACE <ns1> <resource1>... [NAMESPACE <nsX> <resourceX>...]... ]
        INCLUDE_JARS lib/jcommon-1.0.10.jar lib/jfreechart-1.0.6.jar lib/jfreecart-1.0.6-swt.jar lib/swtgraphics2d.jar
        # [ENTRY_POINT <entry>]
        MANIFEST manifest2
        OUTPUT_NAME SoarJavaDebugger
        [OUTPUT_DIR <dir>]
        # [GENERATE_NATIVE_HEADERS <target>
                                #  [DESTINATION (<dir>|INSTALL <dir> [BUILD <dir>])]]
        )
