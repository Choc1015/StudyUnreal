# UPROPERTY를 빼먹으면 생기는 일: 언리얼 GC와 TWeakObjectPtr

## 궁금했던 것

스마트 포인터 단원을 끝내면서 이런 질문을 남겼다.

> 이런 포인터들을 실제 개발 때 언제 쓰는지 궁금하긴 한데, 어차피 하다 보면 나올 테니 넘어가는 걸로.

언리얼의 UObject는 `delete`로 지우지 않는다. 가비지 컬렉터(GC)가 "아무도 안 쓰는 UObject"를 찾아서 지운다. 그럼 GC는 "누가 쓰고 있는지"를 어떻게 알까?

Actor 안에 `UPROPERTY()`가 붙은 포인터와 안 붙은 포인터를 두고, 둘 다 UObject를 가리키게 한 뒤 GC를 돌리면?

실행 전 내 예측:

> ItemA는 쓰지 않을 때 해제되겠지만, ItemB는 해제되지 않고 계속 남아 있는다. 하지만 그걸 아무도 모르게 된다.

> 지워진 쪽 포인터에 남는 건? 쓰레기 값?

## 실험 설계

지워질 때 이름을 찍는 UObject를 만들었다.

```cpp
UCLASS()
class STUDYUNREAL_API UMyItem : public UObject
{
	GENERATED_BODY()
public:
	virtual void BeginDestroy() override;
};

void UMyItem::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(LogTemp, Warning, TEXT("BeginDestroy: %s"), *GetName());
}
```

LifecycleActor에 포인터 두 개를 두고 BeginPlay에서 만들었다.

```cpp
UPROPERTY()
UMyItem* ItemA;    // UPROPERTY 있음

UMyItem* ItemB;    // 그냥 C++ 포인터
```

```cpp
ItemA = NewObject<UMyItem>(this, TEXT("ItemA"));
ItemB = NewObject<UMyItem>(this, TEXT("ItemB"));
```

플레이 중에 콘솔(`~`)에서 `obj gc`를 입력해 GC를 직접 돌리고, 정지했다.

## 결과

### UPROPERTY가 있을 때와 없을 때

```
Cmd: obj gc
Collecting garbage and resetting GC timer.
BeginDestroy: ItemB          ← GC 직후 바로 지워짐
...
EndPlay
BeginDestroy: ItemA          ← 플레이가 끝나고 나서야 지워짐
```

> 음.. 봤는데 GC에서 UPROPERTY가 없으니 그냥 지워 버리네.

예측과 정반대였다. GC는 `UPROPERTY()`가 붙은 포인터만 "쓰고 있다"로 본다. ItemB는 C++ 코드에서는 분명히 가리키고 있지만 GC 눈에는 안 보여서, 아무도 안 쓰는 객체로 보고 바로 지웠다. 그리고 ItemB 변수에는 지워진 주소가 그대로 남는다. 스마트 포인터 단원에서 본 댕글링 포인터다.

### 살려 두지는 않지만, 지워졌는지는 아는 포인터

가리키고는 싶지만 살려 둘 책임은 없을 때는 무엇을 쓸까?

> TObjectWeakPtr

개념은 맞았고, 실제 이름은 `TWeakObjectPtr`이다. 스마트 포인터 단원에서 만든 MyWeak의 언리얼 버전이다.

```cpp
TWeakObjectPtr<UMyItem> WatchA;   // ItemA를 보기만 함
TWeakObjectPtr<UMyItem> ItemC;    // 새 MyItem을 이것만 들고 있음
```

```cpp
ItemA = NewObject<UMyItem>(this, TEXT("ItemA"));
WatchA = ItemA;
ItemC = NewObject<UMyItem>(this, TEXT("ItemC"));
```

EndPlay에서 `IsValid()`로 살아 있는지 물었다.

실행 전 내 예측:

> ItemC는 정지할 때 지워지겠지?

> WatchA yes, ItemC no 나오지 않을까? WatchA는 ItemA가 사라져도 안 사라지니까?

(첫 실행은 `obj gc`를 빼먹어서 다시 했다.)

```
Collecting garbage (obj gc)
BeginDestroy: ItemC          ← GC 직후 지워짐
...
WatchA valid: yes
ItemC valid: no
BeginDestroy: ItemA
```

ItemC는 weak 하나만 들고 있어서 GC 직후 지워졌다. 다만 ItemB와 달리 지워진 걸 알고 `valid: no`를 돌려줬다.

그럼 WatchA도 weak인데 왜 yes였을까?

> UPROPERTY에서 살려 둔 거 아닐까?

맞다. ItemA를 지킨 건 `UPROPERTY() UMyItem* ItemA`였고, WatchA는 그 옆에서 보기만 했다.

## 왜 그런가

| 스마트 포인터 단원 | 언리얼 UObject | GC가 돌면 |
|---|---|---|
| shared_ptr (살려 둠) | `UPROPERTY()` 포인터 | 지켜짐 |
| weak_ptr (보기만, 지워진 걸 앎) | `TWeakObjectPtr` | 지켜 주지 않음, 지워지면 IsValid() false |
| 일반 포인터 | UPROPERTY 없는 포인터 | 지켜 주지 않음, 지워져도 모름 (댕글링) |

- GC는 숫자를 세지 않고, 언리얼 리플렉션이 아는 `UPROPERTY()` 포인터를 따라가며 "쓰는 객체"를 찾는다.
- 그래서 UObject를 들고 있을 때 UPROPERTY를 빼먹으면, 객체가 언제 사라질지 모르는 댕글링 포인터가 된다.
- 다른 Actor를 가리키되 살려 둘 책임이 없을 때(예: 타깃, 길드장)는 `TWeakObjectPtr`를 쓰고, 쓰기 전에 `IsValid()`로 확인한다.

## 정리

> ✏️ 작성 필요: 이 글의 결론을 세 줄로. (예측과 반대였던 것, UPROPERTY가 하는 일, 스마트 포인터 단원과 어떻게 이어지는지 넣으면 좋아요)

## 더 궁금해진 것

> ✏️ 작성 필요: 새로 궁금해진 점 한 가지. 없으면 "없음"이라고 보내 주세요.
