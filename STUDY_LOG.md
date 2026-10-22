# STUDY_LOG (언리얼)

목표: 언리얼 C++ 클라이언트 프로그래머로 취업. C++ 기초는 StudyForFeature 저장소에서 마침 (가상 함수, MyVector, 스마트 포인터).

## 진도

| 과제 | 상태 |
|---|---|
| 언리얼 1: Actor 생명주기 | 완료 (2026-10-22) |
| 언리얼 2: UPROPERTY와 GC | 완료 (2026-10-22) |
| 언리얼 3: 포탑과 대상 (TWeakObjectPtr 실전) | |

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
- 다음 실험: 생성자에서 `GetWorld()`가 있을까?
  - 내 예측: ① CDO 없음 / ② 미리보기 없음 / ③ 레벨에 놓인 것 있음 / ④ 플레이 복사 중 있음 / ⑤ 플레이 있음
  - 실행 결과:
    ```
    ① Default__LifecycleActor          World: no    (예측 맞음)
    ② LifecycleActor_0 (미리보기)       World: yes   (예측 틀림)
    ③ 레벨에 놓인 것                    World: yes   (예측 맞음)
    ④ 플레이 복사 중                    World: no    (예측 틀림)
    ⑤ 플레이                            World: no    (예측 틀림)
    ```
  - 내 반응: 오잉 신기하네
  - 다음 질문: 그럼 월드를 믿고 쓸 수 있는 건 언제부터? PostInitializeComponents와 BeginPlay에서도 World 확인
    - 내 예측: 둘 다 yes
    - 실행 결과 (이번엔 레벨이 저장된 상태로 에디터를 켬):
      ```
      에디터 켜짐   Constructor: Default__LifecycleActor, World: no
      레벨 로드     Constructor: LifecycleActor_1, World: no     ← 저장된 레벨에서 불러올 때도 월드 없음
      플레이        Constructor: LifecycleActor_1, World: no
                    PostInitializeComponents: LifecycleActor_1, World: yes
                    BeginPlay: LifecycleActor_1, World: yes
      정지          Tick 117.94 → EndPlay
      ```
      예측 맞음. 게임용 Actor는 생성자에서는 월드가 없고, PostInitializeComponents부터 있다.
  - 내 해석: 생성자는 초깃값을 써야 하고 레벨 상에 있는 걸 찾는 건 하면 안 된다. 그런 걸 BeginPlay에서 한다.
  - 리뷰: 정확하다. 예시에 대입하면 `Hp = 100;`은 생성자, 다른 Actor 찾기는 BeginPlay, 3초 타이머도 월드의 타이머 관리자를 써야 하니 BeginPlay. 하나 덧붙이면, 생성자에서 하는 대표적인 일은 컴포넌트 만들기(`CreateDefaultSubobject`). 이것도 "초깃값"의 한 종류라 CDO에 들어간다.

**언리얼 1 완료.**

### 언리얼 2: UPROPERTY와 GC

- 출발점 (블로그 14편): 이런 포인터들을 실제 개발 때 언제 쓰는지 궁금하긴 한데, 어차피 하다 보면 나올 테니 넘어가는 걸로.
- 질문: Actor 안에 `UPROPERTY() UMyItem* ItemA;`와 `UMyItem* ItemB;`(UPROPERTY 없음). 둘 다 NewObject로 만들어 두고 GC가 한 번 돌면?
  - 내 예측 1: ItemA는 쓰지 않을 때 해제되겠지만, ItemB는 해제되지 않고 계속 남아 있는다. 하지만 그걸 아무도 모르게 된다.
  - 내 예측 2 (지워진 쪽 포인터에 남는 것): 쓰레기 값?
  - 코드: UMyItem(BeginDestroy에서 이름 로그), LifecycleActor에 `UPROPERTY() UMyItem* ItemA;`, `UMyItem* ItemB;`, BeginPlay에서 `NewObject<UMyItem>(this, TEXT("ItemA"))` / `"ItemB"`
  - 실행 결과 (플레이 중 콘솔에 `obj gc`):
    ```
    12.10.24  Cmd: obj gc
              Collecting garbage and resetting GC timer.
              BeginDestroy: ItemB          ← GC 직후 바로 지워짐
    12.10.29  Tick 118.24 → EndPlay
              BeginDestroy: ItemA          ← 플레이가 끝나 Actor가 사라진 뒤에야 지워짐
    ```
  - 내 해석: 음.. 봤는데 GC에서 UPROPERTY가 없으니 그냥 지워 버리네.
  - 리뷰: 예측과 정반대. GC는 UPROPERTY가 붙은 포인터만 "누가 쓰고 있다"로 본다. ItemB는 C++에서는 분명히 가리키고 있지만 GC 눈에는 아무도 안 쓰는 객체라 바로 지운다. ItemA는 Actor가 살아 있는 동안 지켜진다. 그리고 ItemB 변수에는 여전히 지워진 주소가 남는다 (댕글링 포인터). 예측 2의 "쓰레기 값"은 이 뜻에 가깝다.
- 질문: 가리키고는 싶지만 살려 둘 책임은 없을 때, 지워졌는지는 알 수 있는 포인터는? 언리얼에서는 이름이 뭘까?
  - 내 답: TObjectWeakPtr → 개념은 정확 (weak_ptr). 실제 이름은 `TWeakObjectPtr`.
- 실험: `TWeakObjectPtr<UMyItem> WatchA;`(ItemA를 봄), `TWeakObjectPtr<UMyItem> ItemC;`(새 MyItem을 이것만 듦). 플레이 → obj gc → 정지
  - 내 예측 1 (`BeginDestroy: ItemC`는 언제?): 정지할 때 나오겠지?
  - 내 예측 2 (EndPlay에서 WatchA valid / ItemC valid): yes, no 나오지 않을까? WatchA는 ItemA가 사라져도 안 사라지니까?
  - 첫 실행은 obj gc가 빠져서 다시 함
  - 실행 결과:
    ```
    13.24.23  Collecting garbage (obj gc)
              BeginDestroy: ItemC          ← GC 직후 지워짐 (예측 1은 "정지할 때")
    13.24.29  WatchA valid: yes            ← 예측 2 맞음
              ItemC valid: no              ← 예측 2 맞음
              BeginDestroy: ItemA          ← 정지할 때
    ```
  - 질문: WatchA도 weak인데 왜 yes였을까? 누가 ItemA를 살려 뒀을까?
    - 내 해석: UPROPERTY에서 살려 둔 거 아닐까?
    - 리뷰: 정확하다. ItemA를 지킨 건 `UPROPERTY() UMyItem* ItemA`이고, WatchA는 그 옆에서 보기만 했다. 예측할 때의 이유("WatchA는 ItemA가 사라져도 안 사라지니까")는 거꾸로였다. weak는 대상을 살려 두지 않고, 대상이 사라지면 같이 invalid가 된다.
  - 정리:
    | 스마트 포인터 단원 | 언리얼 UObject | GC가 돌면 |
    |---|---|---|
    | shared_ptr (살려 둠) | `UPROPERTY()` 포인터 | 지켜짐 |
    | weak_ptr (보기만, 지워진 걸 앎) | `TWeakObjectPtr` | 지켜 주지 않음, 지워지면 IsValid() false |
    | 일반 포인터 | UPROPERTY 없는 포인터 | 지켜 주지 않음, 지워져도 모름 (댕글링) |

**언리얼 2 완료.**

### 언리얼 3: 포탑과 대상 (TWeakObjectPtr 실전)

- 출발점 (블로그 16편): 이해는 했는데, 중요한 건 진짜 게임 만들 때 어찌 쓰이는지 과정을 보고 싶다.
- 상황: 포탑(ATurret)이 대상(ATargetDummy)을 계속 바라보며 거리를 잰다. 대상은 3초 뒤 스스로 사라진다. 포탑은 대상을 살려 둘 책임이 없다.
- 1단계: ATargetDummy (BeginPlay에서 3초 수명, 사라질 때 로그)
