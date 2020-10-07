# Physics programing example

Playing around with physics simulations.

## Dependencies
|Tool/Library|Description|
|---|---|
|vcpkg|C++ package management|
|ninja|Build Tool|
|CMake|Build Configuration|
|fmt|String formatting|
|cppitertools|Python-like iteration|
|Catch2|Unit Testing|

## Structure
```
├─── .vscode -| vscode settings
├─── build   -| build output directory
├─── cmake   -| cmake toolchain files
├─── src     -| project sources
└─── test    -| unit testing sources
```

## VSCode settings
```.vscode\c_cpp_properties.json``` for C\C++ tools extension 
```json
{
    "configurations": [
        {
            ...,
            "defines": [
                "_DEBUG",
                "DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            ...,
            "cppStandard": "c++20",
            ...,
            "compilerArgs": [
                "/permissive-",
                "/W3",
                "/Zc:__cplusplus"
            ],
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    ...
}
```
```launch.json``` for debugger in VSCode
```json
{
    ...,
    "configurations": [
        {
            "name": "(Windows) Launch",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}\\build\\bin",
            "environment": [],
            "externalConsole": false
        }
    ]
}
```