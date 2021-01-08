# ROSTRviewer hotfix
Enables ROSTRviewer to:
* Load GRF archives over 2GiB
* Load *foldered* effects.

## Install
* Compile DLL as hotfix32.dll
* Replace kernel32.dll import that contains SetFilePointer with hotfix32.dll
