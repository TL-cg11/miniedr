\# Mini EDR



Windows 환경용 경량 EDR — YARA 룰과 해시 블랙리스트로 파일을 실시간 탐지하고 자동 격리하는 학습용 프로젝트입니다.



\## ✨ Features



\- YARA 룰 기반 정적 탐지 — 파일 내용 패턴 매칭

\- SHA256 해시 블랙리스트 매칭 — Windows CNG API 사용

\- 폴더 실시간 감시 — `ReadDirectoryChangesW` 비동기 I/O + 별도 스레드

\- 탐지 시 자동 격리 — `MoveFileExW` 기반, 원본 보존 정책

\- SQLite + JSON Lines 이중 이벤트 저장 — 분석 / SIEM 호환

\- 한글 파일명 및 경로 완전 지원 — UTF-8 일관성

\- CLI 기반 — `scan` (온디맨드) / `monitor` (실시간) 두 모드



\## 🛠️ Requirements



| 항목 | 버전 / 사양 |

|------|-----------|

| OS | Windows 10 / 11 (x64) |

| 컴파일러 | MSVC (Visual Studio 2022 권장) |

| CMake | 3.20 이상 |

| vcpkg | 의존성 관리용 |



\### 의존성 (vcpkg로 자동 설치)



\- \[YARA](https://github.com/VirusTotal/yara) — 정적 탐지 엔진

\- \[spdlog](https://github.com/gabime/spdlog) — 로깅

\- \[SQLite3](https://www.sqlite.org/) — 이벤트 DB

\- \[nlohmann/json](https://github.com/nlohmann/json) — JSON Lines 직렬화



\## 🚀 Quick Start



\### 1. 저장소 클론 + 빌드



```powershell

git clone https://github.com/TL-cg11/miniedr.git miniedr

cd miniedr



\# 방법 A: vcpkg integrate install 된 환경

cmake -B build -S .



\# 방법 B: 수동 (vcpkg 경로 명시)

cmake -B build -S . -DCMAKE\_TOOLCHAIN\_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake



cmake --build build --config Debug

```



빌드 산출물: `build/Debug/miniedr.exe`



\### 2. 테스트 폴더 + EICAR 준비



```powershell

mkdir C:\\test\\watch



\# EICAR 표준 테스트 파일 생성 (실제 악성코드 아님, 모든 AV가 인식하는 더미)

$eicar = 'X5O!P%@AP\[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H\*'

\[System.IO.File]::WriteAllText("C:\\test\\watch\\eicar.com", $eicar, \[System.Text.UTF8Encoding]::new($false))

```



\### 3. 스캔 실행



```powershell

cd build\\Debug

.\\miniedr.exe scan C:\\test\\watch --rules ..\\..\\rules\\test.yar --hashes hashes.txt

```



\### 4. 결과 확인



\- `quarantine\\` 폴더에 `.qtn` 파일이 생성됨 → 격리 성공

\- `events.db` (SQLite) 에 탐지 이벤트 1건 기록

\- `events.jsonl` 에 JSON Lines 한 줄 추가

\- 원본 `C:\\test\\watch\\eicar.com` 는 사라짐



\## 📖 Usage



Mini EDR 은 두 가지 모드를 지원합니다.



\### Scan 모드 — 온디맨드 1회 스캔



지정한 폴더를 재귀적으로 탐색하여 한 번에 검사합니다.



```powershell

miniedr.exe scan <대상폴더> --rules <yara룰파일> --hashes <해시리스트파일>

```



\*\*예시:\*\*

```powershell

miniedr.exe scan C:\\test\\watch --rules ..\\..\\rules\\test.yar --hashes hashes.txt

```



\*\*기대 출력:\*\*

```

\[info] Database initialized

\[info] Quarantine: ready at 'quarantine'

\[info] Hash Blacklist loaded successfully

\[info] YARA rules loaded successfully

\[info] FileScanner found 4 files

\[warning] HashDetector matched rule 'BlacklistHash:...' on file C:/test/watch/eicar.com

\[info] Quarantine: 'C:/test/watch/eicar.com' -> 'quarantine\\20260524\_192232\_eicar.com.qtn'

```



\### Monitor 모드 — 실시간 폴더 감시



지정한 폴더를 백그라운드 스레드로 감시합니다. 파일 생성·수정 이벤트가 발생하면 자동으로 검사합니다. 종료 시 콘솔에 `stop` 을 입력하세요.



```powershell

miniedr.exe monitor <대상폴더> --rules <yara룰파일> --hashes <해시리스트파일>

```



다른 콘솔에서 `C:\\test\\watch\\` 안에 EICAR 파일을 떨어뜨리면 즉시 탐지·격리됩니다.



\### CLI 인자



| 인자 | 필수 | 설명 |

|------|------|------|

| `scan <폴더>` | ✓ | 1회 스캔 모드 |

| `monitor <폴더>` | ✓ | 실시간 감시 모드 |

| `--rules <파일>` | ✓ | YARA 룰 파일 경로 |

| `--hashes <파일>` | ✓ | 해시 블랙리스트 파일 경로 |



\### Hashes 파일 포맷



\- 한 줄에 SHA256 해시 하나 (16진수 64자)

\- 대소문자 무관 (내부에서 소문자로 정규화)

\- 줄 앞뒤 공백 / 탭 자동 제거

\- 빈 줄은 자동 스킵

\- Windows 줄바꿈(`\\r\\n`) / Unix 줄바꿈(`\\n`) 모두 지원

\- \*\*주석 미지원\*\* — `#` 또는 `//` 로 시작해도 해시로 간주됨 (향후 개선 예정)

\- \*\*형식 검증 없음\*\* — 잘못된 해시도 로딩됨 (매칭 실패만 발생)



\*\*예시 (`hashes.txt`):\*\*

```

275a021bbfb6489e54d471899f7db9d1663fc695ec2fe2a2c4538aabf651fd0f

```



\## 🧪 Test Scenarios



EICAR 표준 테스트 파일로 핵심 시나리오를 설계·검증했습니다. 본격 회귀 테스트는 보고서 작성 시점에 일괄 수행 예정입니다.



\### Scan 모드



| # | 시나리오 | 기대 결과 | 상태 |

|---|---------|----------|------|

| S2 | 정상 파일 3개 폴더 스캔 | 탐지 0건, 격리 없음 (오탐 검증) | 📋 보류 |

| S3 | EICAR 1개 + 정상 3개 폴더 스캔 | EICAR 탐지·격리, 정상 파일 보존 | ✅ 통과 |

| S5 | 한글 파일명 EICAR 스캔 | 인코딩 문제 없이 탐지·격리 | 📋 보류 |



\### Monitor 모드



| # | 시나리오 | 기대 결과 | 상태 |

|---|---------|----------|------|

| M1 | 빈 감시 폴더에 EICAR 투입 | FileCreated → 자동 스캔 → 탐지·격리 | 📋 보류 |

| M3 | 정상 파일 내용을 EICAR 로 수정 | FileModified → 자동 스캔 → 탐지·격리 | 📋 보류 |

| M4 | `stop` 입력 시 종료 | 좀비 스레드 없이 깨끗하게 종료 | 📋 보류 |



\### 권한 / 에러 처리



| # | 시나리오 | 기대 결과 | 상태 |

|---|---------|----------|------|

| E1 | `C:\\Windows\\System32\\config` 스캔 | 권한 거부 폴더 스킵, 크래시 없음 | ✅ 통과 |



> ✅ = 검증 완료, 📋 보류 = 정의는 완료, 본격 회귀 테스트는 보고서 작성 시점에 수행



\## 📁 Project Structure



```

miniedr/

├── CMakeLists.txt

├── vcpkg.json

├── README.md

│

├── src/

│   ├── main.cpp                    # 진입점, EventBus 조립, 모드 분기

│   ├── cli/

│   │   └── Args.hpp / .cpp         # CLI 인자 파싱 (scan / monitor + 옵션)

│   ├── core/

│   │   ├── Event.hpp               # 이벤트 타입 정의

│   │   ├── EventBus.hpp / .cpp     # Pub-Sub 메시지 버스

│   │   └── Logger.hpp / .cpp       # spdlog 래퍼

│   ├── detectors/

│   │   ├── IDetector.hpp           # 탐지기 인터페이스

│   │   ├── YaraDetector.hpp / .cpp # YARA 룰 기반 탐지

│   │   └── HashDetector.hpp / .cpp # SHA256 블랙리스트 탐지

│   ├── scanner/

│   │   ├── FileScanner.hpp / .cpp       # 폴더 재귀 탐색

│   │   └── DirectoryMonitor.hpp / .cpp  # 실시간 감시 (ReadDirectoryChangesW)

│   ├── quarantine/

│   │   └── Quarantine.hpp / .cpp   # 격리 처리 (MoveFileExW)

│   └── storage/

│       └── Database.hpp / .cpp     # SQLite 이벤트 저장

│

├── rules/

│   └── test.yar                    # EICAR 탐지 룰

│

└── 실행 시 자동 생성 (build/Debug/ 작업 디렉터리):

&#x20;   ├── hashes.txt                  # SHA256 블랙리스트 (수동 작성)

&#x20;   ├── quarantine/                 # 격리된 파일 (.qtn)

&#x20;   ├── events.db                   # SQLite 이벤트 DB

&#x20;   └── events.jsonl                # JSON Lines 이벤트 로그

```



\### 핵심 모듈 흐름



```

\[CLI 인자] → Args 파싱 → main.cpp → EventBus 조립

&#x20;                                    │

&#x20;                             ┌──────┼──────┐

&#x20;                             ▼      ▼      ▼

&#x20;                       FileScanner  Detectors  DirectoryMonitor

&#x20;                       (scan)       (YARA+Hash) (monitor, 별도 스레드)

&#x20;                             │      │      │

&#x20;                             └──────┼──────┘

&#x20;                                    ▼

&#x20;                             ThreatDetected Event

&#x20;                                    │

&#x20;                         ┌──────────┼──────────┐

&#x20;                         ▼          ▼          ▼

&#x20;                     Quarantine   Database   JSON Lines

&#x20;                     (.qtn 이동)  (events.db) (events.jsonl)

```



\## ⚠️ Known Limitations



학습용 1차 프로젝트로서 다음 한계가 있습니다.



\### 탐지 범위

\- \*\*정적 탐지만 지원\*\* — 파일 내용 기반 패턴 매칭 / 해시 비교만 수행

\- 동적 행위 탐지 미지원 — 프로세스 실행, 레지스트리 변경, 네트워크 트래픽 모니터링 등은 2차 프로젝트 범위

\- 커널 레벨 후킹 미지원 — 사용자 모드(Win32 API) 에서만 동작

\- 압축 파일(zip, rar) 내부 미스캔 — 압축 해제 없이 원시 바이너리만 검사



\### Response 범위

\- 격리만 지원 — 프로세스 강제 종료, 네트워크 차단, 자동 복구 등은 미지원

\- 격리 파일 복원 기능 없음 — `.qtn` 파일은 수동으로 원래 위치로 이동해야 복원



\### 운영 한계

\- 단일 폴더 감시 — Monitor 모드는 한 번에 하나의 폴더만 감시 (멀티 폴더는 잔여 빚)

\- 권한 거부 폴더는 사일런트 스킵 — `skip\_permission\_denied` 옵션 사용으로 누락 카운트가 별도 노출되지 않음

\- 룰 / 해시 핫 리로드 미지원 — 룰 파일 변경 시 EDR 재시작 필요

\- 다중 인스턴스 안전성 미검증 — 동일 폴더를 두 인스턴스가 감시 시 동작 미정의



\### 보안 자체의 한계

\- EDR 자체 자기 보호(self-protection) 없음 — 악성 프로세스가 EDR 종료 가능

\- 격리 폴더 ACL 미적용 — 일반 사용자도 `.qtn` 파일 접근 가능

\- 코드 서명 / TLS / 통신 암호화 없음 — 로컬 단독 동작



\## 📜 Acknowledgements



본 프로젝트는 다음 오픈소스 라이브러리를 사용합니다.



| 라이브러리 | 역할 | 라이선스 |

|----------|------|---------|

| \[YARA](https://github.com/VirusTotal/yara) | 정적 탐지 엔진 | BSD 3-Clause |

| \[spdlog](https://github.com/gabime/spdlog) | 로깅 | MIT |

| \[SQLite3](https://www.sqlite.org/) | 이벤트 DB | Public Domain |

| \[nlohmann/json](https://github.com/nlohmann/json) | JSON 직렬화 | MIT |



\### 학습 참고



\- \[OpenEDR](https://github.com/ComodoSecurity/openedr) — 오픈소스 EDR 아키텍처 학습용

\- \[EICAR](https://www.eicar.org/) — 표준 AV 테스트 파일 명세



\---



\*이 프로젝트는 EDR 의 핵심 동작 원리를 학습하기 위한 1차 프로젝트 결과물입니다.\*

