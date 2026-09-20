@file:Suppress("UnstableApiUsage")

plugins {
    id ("com.android.application")
}

android {
    namespace = "ru.bluecat.gpaycountryspoofer"
    compileSdk = 37
    ndkVersion = "30.0.16248370"

    defaultConfig {
        applicationId = "ru.bluecat.gpaycountryspoofer"
        minSdk = 26
        targetSdk = 37
        versionCode = 3
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                val flags = arrayOf(
                    "-fvisibility=hidden",
                    "-fvisibility-inlines-hidden",
                    "-fno-exceptions",
                    "-fno-rtti"
                )
                cppFlags("-std=c++23", *flags)
                cFlags("-std=c18", *flags)
            }
        }
        ndk.abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a"))
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true

            externalNativeBuild {
                cmake {
                    arguments(
                        "-DCMAKE_BUILD_TYPE=Release",
                        "-DCMAKE_CXX_FLAGS_RELEASE=-Oz",
                        "-DCMAKE_C_FLAGS_RELEASE=-Oz",
                        "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,--gc-sections -Wl,--exclude-libs,ALL -Wl,--strip-all"
                    )
                    val releaseFlags = arrayOf(
                        "-ffunction-sections",
                        "-fdata-sections"
                    )
                    cppFlags += releaseFlags
                    cFlags += releaseFlags
                }
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "4.1.2"
        }
    }

    buildFeatures {
        prefab = true
    }
}

val copyFiles: Any = tasks.register("copyFiles") {
    description = "Copying libraries for archiving"

    val moduleFolder = project.rootDir.resolve("module")
    val soDir = project.layout.buildDirectory.get().asFile.resolve("intermediates/stripped_native_libs/release/stripReleaseDebugSymbols/out/lib")

    doLast {
        soDir.walk().filter { it.isFile && it.extension == "so" }.forEach { soFile ->
            val abiFolder = soFile.parentFile.name
            val destination = moduleFolder.resolve("zygisk/$abiFolder.so")
            soFile.copyTo(destination, overwrite = true)
        }
    }
}

val zipFiles: Any = tasks.register<Zip>("zipFiles") {
    description = "Creating the module archive"
    dependsOn("copyFiles")

    val versionName = android.defaultConfig.versionName
    archiveFileName.set("Gpay Country Spoofer v$versionName.zip")
    destinationDirectory.set(project.rootDir.resolve("out"))

    from(project.rootDir.resolve("module"))
}

tasks.whenTaskAdded {
    if (name == "assembleRelease") finalizedBy(copyFiles, zipFiles)
}