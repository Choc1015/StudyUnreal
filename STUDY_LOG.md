# STUDY_LOG (언리얼)

목표: 언리얼 C++ 클라이언트 프로그래머로 취업. C++ 기초는 StudyForFeature 저장소에서 마침 (가상 함수, MyVector, 스마트 포인터).

## 진도

| 과제 | 상태 |
|---|---|
| 언리얼 1: Actor 생명주기 | |

---

## 실험 기록

### 언리얼 1: Actor 생명주기

- 출발점 (처음 자가진단 8번): 생명주기가 있다는 거와 beginplay 전에 작동하는 함수가 있다는 등 어느 정도 인지는 하지만 설명하라고 하면 어렵다.
- 내 예측 (함수가 불리는 순서): 생성자 → PostInitializeComponents → BeginPlay → Tick → EndPlay
  - 생성자는 한 번만 불릴까? → 아니, 에디터 상에 놓일 때 한 번이랑 플레이 누를 때?
  - Tick은 몇 번? → 1초에 60번
- 코드: `ALifecycleActor` (생성자, PostInitializeComponents, BeginPlay, EndPlay에 UE_LOG, Tick은 횟수만 세서 EndPlay에서 `num / GetTimeSeconds()` 출력)
- 실행 결과 (출력 로그에서 복사):
  ```
  ALifecycleActor
  ALifecycleActor
  PostInitializeComponents
  BeginPlay
  Tick 115.467026
  EndPlay
  ```
- 내 해석: tick은 n번을 초로 나눈 건데, 보니까 내 컴퓨터에 맞는 성능 fps가 나온 거 같고 나머지는 예상대로 나온 거 같아.
- 리뷰:
  - Tick 해석은 정확하다. Tick은 "1초에 60번"이 아니라 **프레임마다 한 번**이라, 초당 횟수 = 그 컴퓨터의 FPS. 그래서 Tick 안의 움직임은 DeltaTime을 곱해야 컴퓨터마다 같은 속도가 된다.
  - 생성자는 예상보다 훨씬 많이 불렸다. 전체 로그 파일(`Saved/Logs/StudyUnreal.log`)을 시간순으로 보면:
    ```
    08.42.34  에디터 켜짐 (프레임 0, 레벨에 아무것도 없음)  → ALifecycleActor
    08.43.17  Actor를 뷰포트로 끌고 오는 중               → ALifecycleActor
    08.43.18  놓음 (사이에 "Deleted 0 Actors")            → ALifecycleActor
    08.43.26  플레이: "Creating play world package" 직후 → ALifecycleActor
    08.43.26  플레이: PostInitializeComponents 바로 앞  → ALifecycleActor
              → PostInitializeComponents → BeginPlay
    ```
    생성자 5번, PostInitializeComponents와 BeginPlay는 각 1번. 각 생성자가 무엇 때문인지는 다음 실험에서 이름을 찍어 확인한다.
- 생성자에 `GetName()` 추가
  - 첫 빌드 에러: `static_assert failed: 'missing format specifier'` → 로그 문자열에 한글을 넣었는데 파일이 CP949로 저장되어, UTF-8로 컴파일할 때 깨져서 %s를 못 찾음. 로그 문자열은 영어로.
  - 결과 (에디터 켜기 → 끌어다 놓기 → 플레이 → 정지):
    ```
    09.23.53  에디터 켜짐            Constructor: Default__LifecycleActor
    09.25.05  끌고 오는 중            Constructor: LifecycleActor_0
    09.25.05  놓음                    Constructor: LifecycleActor_UAID_D843AE25E2BAA40D03_2040172423
    09.25.14  플레이 (레벨 복사 중)   Constructor: LifecycleActor_UAID_..._2040172423
    09.25.14  플레이                  Constructor: LifecycleActor_UAID_..._2040172423
                                      PostInitializeComponents → BeginPlay
              정지                    Tick 116.57 → EndPlay
    ```
  - 질문: 레벨에 놓기도 전에 에디터를 켜자마자 만들어진 `Default__LifecycleActor`는 무엇일까?
    - 내 답: 음... 에디터 자체도 실행할 때 안에 있는 파일들을 한 번 구우니까 생성자가 실행되는 느낌?
    - 리뷰: "실행할 때 한 번 만든다"는 맞다. 다만 "굽는다(쿠킹)"는 배포용 패키지를 만드는 다른 과정이다. 이건 **CDO(Class Default Object)**: 언리얼이 UCLASS마다 기본값 견본을 하나씩 만들어 두는 것. 디테일 패널의 기본값, 새로 놓는 Actor의 초기값이 여기서 복사된다. 그래서 생성자는 레벨도 게임도 없는 상태에서도 불린다.
