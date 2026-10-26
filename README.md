# Input Assist (Test)

테스트용 입력 보조 모드입니다.
- 클릭을 놓는 시점을 최소 N스텝(기본 4) 뒤로 미뤄서 판정 허용 범위를 넓힙니다.
- 켜져 있는 동안 화면 위쪽에 "INPUT ASSIST ON - NO RECORDS"가 표시됩니다.
- 보조가 사용된 플레이는 레벨 완료 기록이 저장되지 않습니다.

## 빌드 (Visual Studio 없이): GitHub Actions
1. GitHub 가입 후 새 저장소(Repository) 생성
2. 이 폴더의 모든 파일(.github 폴더 포함)을 저장소에 업로드
3. 저장소 상단 Actions 탭에서 "Build Geode Mod" 실행 완료까지 대기 (3~10분)
4. 완료된 실행 하단 Artifacts의 "Build Output" 다운로드
5. 압축을 풀어 나온 .geode 파일을 Geometry Dash의 geode/mods 폴더에 넣기

mod.json의 geode 버전은 5.11.2로 맞춰져 있습니다.
