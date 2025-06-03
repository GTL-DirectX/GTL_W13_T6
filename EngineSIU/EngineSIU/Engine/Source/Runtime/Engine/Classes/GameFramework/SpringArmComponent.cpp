#include "SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Launch/EngineLoop.h"
#include "World/World.h"

USpringArmComponent::USpringArmComponent()
{

}

UObject* USpringArmComponent::Duplicate(UObject* InOuter)
{
	ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
	NewComponent->TargetArmLength = TargetArmLength;
	NewComponent->SocketOffset = SocketOffset;
	NewComponent->MinPitch = MinPitch;
	NewComponent->MaxPitch = MaxPitch;
	NewComponent->MinYaw = MinYaw;
	NewComponent->MaxYaw = MaxYaw;
	return NewComponent;
}

void USpringArmComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
	Super::GetProperties(OutProperties);
	// 카메라의 FOV, AspectRatio, NearClip, FarClip을 OutProperties에 추가
	OutProperties.Add(TEXT("TargetArmLength"), FString::Printf(TEXT("%f"), TargetArmLength));
	OutProperties.Add(TEXT("SocketOffset"), *SocketOffset.ToString());
	OutProperties.Add(TEXT("MinPitch"), FString::Printf(TEXT("%f"), MinPitch));
	OutProperties.Add(TEXT("MaxPitch"), FString::Printf(TEXT("%f"), MaxPitch));
	OutProperties.Add(TEXT("MinYaw"), FString::Printf(TEXT("%f"), MinYaw));
	OutProperties.Add(TEXT("MaxYaw"), FString::Printf(TEXT("%f"), MaxYaw));
}

void USpringArmComponent::SetProperties(const TMap<FString, FString>& InProperties)
{// 카메라의 FOV, AspectRatio, NearClip, FarClip을 InProperties에서 읽어와 설정
	Super::SetProperties(InProperties);
	const FString* TempStr = nullptr;
	TempStr = InProperties.Find(TEXT("TargetArmLength"));
	if (TempStr) TargetArmLength = FCString::Atof(**TempStr);
	TempStr = InProperties.Find(TEXT("SocketOffset"));
	if (TempStr) SocketOffset.InitFromString(*TempStr);
	TempStr = InProperties.Find(TEXT("MinPitch"));
	if (TempStr) MinPitch = FCString::Atof(**TempStr);
	TempStr = InProperties.Find(TEXT("MaxPitch"));
	if (TempStr) MaxPitch = FCString::Atof(**TempStr);
	TempStr = InProperties.Find(TEXT("MinYaw"));
	if (TempStr) MinYaw = FCString::Atof(**TempStr);
	TempStr = InProperties.Find(TEXT("MaxYaw"));
	if (TempStr) MaxYaw = FCString::Atof(**TempStr);
}

void USpringArmComponent::InitializeComponent()
{
	// !TODO : Input 시스템 찾아서 바인드
	Super::InitializeComponent();

}

void USpringArmComponent::BeginPlay()
{
	// SpringArmComponent는 BeginPlay에서 카메라 찾거나 생성
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(ELogLevel::Error, TEXT("SpringArmComponent With No Owner"));
		return;
	}
	Camera = Owner->GetComponentByClass<UCameraComponent>();
	bIsCameraAttached = Camera != nullptr;

	// 에디터에서는 인풋바인딩 하지 않는다
	UWorld* World = GetWorld();
	if (World && World->WorldType != EWorldType::Editor)
	{
		/*MouseInputHandle = GEngineLoop.GetAppMessageHandler()->OnRawMouseInputDelegate.AddUObject(this, &USpringArmComponent::OnRawMouseInput);*/
	}
}

void USpringArmComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MouseInputHandle.has_value() && MouseInputHandle->IsValid())
	{
		/*GEngineLoop.GetAppMessageHandler()->OnRawMouseInputDelegate.Remove(*MouseInputHandle);
		MouseInputHandle.reset();*/
	}
}

void USpringArmComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
	UpdateCameraTransform(DeltaTime);
	//Camera->LookAt
}

void USpringArmComponent::SetTargetArmLength(float InLength)
{
	TargetArmLength = InLength;
}

float USpringArmComponent::GetTargetArmLength() const
{
	return TargetArmLength;
}

void USpringArmComponent::SetSocketOffset(const FVector& InOffset)
{
	SocketOffset = InOffset;
}

FVector USpringArmComponent::GetSocketOffset() const
{
	return SocketOffset;
}

FVector USpringArmComponent::GetCameraForwardVector()
{
	if (Camera)
	{
		FVector CameraForwardVector = Camera->GetForwardVector();
		CameraForwardVector.Z = 0;
		CameraForwardVector.Normalize();
		return CameraForwardVector;
	}
	return GetForwardVector(); // 카메라가 없을 경우 기본값 반환
}

FVector USpringArmComponent::GetCameraRightVector()
{
	if (Camera)
	{
		FVector CameraRightVector = Camera->GetRightVector();
		CameraRightVector.Z = 0;
		CameraRightVector.Normalize();
		return CameraRightVector;
	}
	return GetRightVector(); // 카메라가 없을 경우 기본값 반환
}

void USpringArmComponent::AttachCamera(UCameraComponent* InCamera)
{
	if (InCamera == nullptr)
		return;

	if (Camera != InCamera)
	{
		//원래 카메라와 다른 카메라가 들어오면 기존 카메라 날리고 새로운 카메라로 세팅
		GetOwner()->RemoveOwnedComponent(Camera);
		// !TODO : 기존재하는 컴포넌트를 액터에 붙이는 API 필요함
		//GetOwner()->AddComponent(InCamera);
		//Camera = InCamera;

		//Camera->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}

	bIsCameraAttached = true;
}

void USpringArmComponent::DetachCamera()
{
	// 플래그만 false처리해주고, 원래 카메라 참조는 유지
	bIsCameraAttached = false;
}

void USpringArmComponent::OnRawMouseInput(const FPointerEvent& InEvent)
{
	FVector2D MouseDelta = InEvent.GetCursorDelta();
	HandleRotation(MouseDelta);
}

void USpringArmComponent::HandleRotation(const FVector2D& Vector)
{
	if (!Camera)
		return;
	//return;
	float yaw = Vector.X;
	float pitch = Vector.Y;

	CurrentPitchAngle = FMath::Clamp(CurrentPitchAngle + pitch / 10.f, MinPitch, MaxPitch);
	CurrentYawAngle = FMath::Clamp(CurrentYawAngle + yaw / 10.f, MinYaw, MaxYaw);
}

void USpringArmComponent::UpdateCameraTransform(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Camera || !bIsCameraAttached)
		return;

	FVector BaseLocation = Owner->GetActorLocation();

	// 회전 계산: Owner 기준으로 Yaw, Pitch 적용
	FRotator CameraRotation(0.f, CurrentYawAngle, 0.f); // Yaw만 먼저 적용
	FVector YawDirection = CameraRotation.ToVector(); // Forward 벡터

	FVector OffsetFromOwner =
		-YawDirection * TargetArmLength * FMath::Cos(FMath::DegreesToRadians(CurrentPitchAngle)) +
		FVector(0, 0, FMath::Sin(FMath::DegreesToRadians(CurrentPitchAngle)) * TargetArmLength);

	FVector DesiredLocation = BaseLocation + OffsetFromOwner + SocketOffset;
	TargetLocation = FMath::Lerp(DesiredLocation, TargetLocation, DeltaTime);

	FVector LookDirection = BaseLocation - TargetLocation;
	LookDirection.Normalize();
	FRotator TargetLookRotation = FRotator::MakeLookAtRotation(TargetLocation,BaseLocation);

	Camera->SetWorldLocation(TargetLocation);
	Camera->SetWorldRotation(TargetLookRotation);
}
