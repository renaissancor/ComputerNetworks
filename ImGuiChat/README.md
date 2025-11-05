# 1. C 드라이브 루트로 이동
cd C:\

# 2. 공식 레포 클론
git clone https://github.com/microsoft/vcpkg.git

# 3. 부트스트랩
cd vcpkg
.\bootstrap-vcpkg.bat

# 4. 전역 통합 (Visual Studio 자동 인식)
.\vcpkg integrate install
