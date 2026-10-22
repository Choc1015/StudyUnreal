# 생성자는 몇 번 불릴까? Actor 생명주기를 직접 찍어 봤다

## 궁금했던 것

언리얼 Actor의 생명주기는 들어 봤지만 설명하라고 하면 어려웠다.

> 생명주기가 있다는 거와 BeginPlay 전에 작동하는 함수가 있다는 등 어느 정도 인지는 하지만 설명하라고 하면 어렵다.

레벨에 Actor 하나를 놓고 플레이했다가 정지하면, 함수들이 어떤 순서로 불릴까?

실행 전 내 예측:

> 생성자 → PostInitializeComponents → BeginPlay → Tick → EndPlay

> 생성자는 한 번만 불릴까? → 아니, 에디터 상에 놓일 때 한 번이랑 플레이 누를 때?

> Tick은 몇 번? → 1초에 60번

## 실험 설계

`ALifecycleActor`를 만들고 생성자, `PostInitializeComponents`, `BeginPlay`, `EndPlay`에서 로그를 찍었다. Tick은 매번 찍으면 넘치니까 횟수만 세고, `EndPlay`에서 플레이 시간으로 나눠 출력했다.

```cpp
ALifecycleActor::ALifecycleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Warning, TEXT("Constructor: %s, World: %s"), *GetName(), GetWorld() ? TEXT("yes") : TEXT("no"));
}

void ALifecycleActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("PostInitializeComponents: %s, World: %s"), *GetName(), GetWorld() ? TEXT("yes") : TEXT("no"));
}

void ALifecycleActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay: %s, World: %s"), *GetName(), GetWorld() ? TEXT("yes") : TEXT("no"));
}

void ALifecycleActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UE_LOG(LogTemp, Warning, TEXT("Tick %f"), num / GetWorld()->GetTimeSeconds());
	UE_LOG(LogTemp, Warning, TEXT("EndPlay"));
}

void ALifecycleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	num += 1;
}
```

(처음엔 로그 문자열에 한글을 넣었다가 `missing format specifier` 빌드 에러가 났다. 파일이 CP949로 저장되어 있는데 언리얼은 UTF-8로 컴파일해서, 한글이 깨지면서 `%s`를 못 찾은 것이었다. 로그 문자열은 영어로.)

## 결과

### 순서와 Tick

```
ALifecycleActor
ALifecycleActor
PostInitializeComponents
BeginPlay
Tick 115.467026
EndPlay
```

> tick은 n번을 초로 나눈 건데, 보니까 내 컴퓨터에 맞는 성능 fps가 나온 거 같고 나머지는 예상대로 나온 거 같아.

Tick은 1초에 60번이 아니라 프레임마다 한 번이다. 그래서 초당 횟수는 그 컴퓨터의 FPS가 된다.

### 생성자는 몇 번?

출력 로그 말고 전체 로그 파일(`Saved/Logs`)을 시간순으로 보니 생성자는 2번이 아니었다. 이름을 찍어 보니 이렇게 나왔다.

```
에디터 켜짐            Constructor: Default__LifecycleActor
끌고 오는 중            Constructor: LifecycleActor_0
놓음                    Constructor: LifecycleActor_UAID_..._2040172423
플레이 (레벨 복사 중)   Constructor: LifecycleActor_UAID_..._2040172423
플레이                  Constructor: LifecycleActor_UAID_..._2040172423
                        PostInitializeComponents → BeginPlay
```

생성자는 다섯 번, `PostInitializeComponents`와 `BeginPlay`는 한 번이었다. 플레이를 누르면 에디터 레벨을 복사해서 게임용 레벨을 따로 만들기 때문에, 레벨에 놓인 것과 같은 이름의 복사본이 생긴다.

### 레벨에 놓기도 전에 만들어진 Default__

> 음... 에디터 자체도 실행할 때 안에 있는 파일들을 한 번 구우니까 생성자가 실행되는 느낌?

실행할 때 한 번 만드는 건 맞다. 이건 CDO(Class Default Object)다. 언리얼은 UCLASS마다 기본값 견본을 하나씩 만들어 두고, 디테일 패널의 기본값이나 새로 놓는 Actor의 초기값을 여기서 복사한다. ("굽는다(쿠킹)"는 배포용 패키지를 만드는 다른 과정이다.)

### 생성자에서 월드가 있을까?

내 예측과 실제 결과:

```
① Default__ (CDO)        예측: 없음   결과: no
② 미리보기               예측: 없음   결과: yes
③ 레벨에 놓인 것          예측: 있음   결과: yes
④ 플레이 복사 중          예측: 있음   결과: no
⑤ 플레이                  예측: 있음   결과: no
```

> 오잉 신기하네

진짜 게임에서 쓰이는 Actor인데도 생성자 안에서는 월드가 없었다. 그럼 언제부터 월드를 믿고 쓸 수 있을까?

> 둘 다 yes

```
레벨 로드     Constructor: LifecycleActor_1, World: no
플레이        Constructor: LifecycleActor_1, World: no
              PostInitializeComponents: LifecycleActor_1, World: yes
              BeginPlay: LifecycleActor_1, World: yes
```

저장된 레벨을 불러올 때도 생성자에는 월드가 없었다. 월드는 `PostInitializeComponents`부터 있다.

## 왜 그런가

- 생성자는 "게임이 시작될 때"가 아니라 "객체가 만들어질 때"마다 불린다. CDO, 에디터 미리보기, 레벨에 놓기, 레벨 불러오기, 플레이용 복사가 전부 객체를 만드는 일이라 생성자가 여러 번 불린다.
- 그중 상당수는 월드가 없거나, 게임과 상관없는 객체다. 그래서 생성자에서 월드나 다른 Actor를 쓰면 안 된다.
- `PostInitializeComponents`와 `BeginPlay`는 게임용 Actor에서 한 번만 불리고, 이때는 월드가 있다.

내 해석:

> 생성자는 초깃값을 써야 하고 레벨 상에 있는 걸 찾는 건 하면 안 된다. 그런 걸 BeginPlay에서 한다.

## 정리

대략적인 플로우를 알고 있긴 했는데, 이렇게 해 보니 정확히 왜 생성자에서 레벨에 대한 정보를 불러오지 않는지 알았다.
