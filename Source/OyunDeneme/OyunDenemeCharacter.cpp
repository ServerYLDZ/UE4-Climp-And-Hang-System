// Copyright Epic Games, Inc. All Rights Reserved.

#include "OyunDenemeCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AOyunDenemeCharacter

AOyunDenemeCharacter::AOyunDenemeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	Climbing = false;
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
	/*
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SetWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	
	*/
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; 
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 5.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;
	CameraLong = 0;
	CameraHeight = 0;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOyunDenemeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOyunDenemeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOyunDenemeCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	//PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis("TurnRate", this, &AOyunDenemeCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &AOyunDenemeCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AOyunDenemeCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AOyunDenemeCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AOyunDenemeCharacter::OnResetVR);
}


void AOyunDenemeCharacter::BeginPlay()
{
	Super::BeginPlay();
	/*APlayerController* MyController = Cast<APlayerController>(GetController());
	if (MyController) {
		MyController->bShowMouseCursor = true;
		MyController->bEnableMouseOverEvents = true;
		MyController->bEnableClickEvents = true;
	}*/
}

void AOyunDenemeCharacter::OnResetVR()
{
	// If OyunDeneme is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in OyunDeneme.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AOyunDenemeCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AOyunDenemeCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AOyunDenemeCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate );
}

void AOyunDenemeCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate);
}

void AOyunDenemeCharacter::MoveForward(float Value)
{

	if ((Controller != nullptr) && (Value != 0.0f))
	{	
		
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		if (!Climbing) {
			
			if (Value < 0 ) {
				CameraHeight = CameraHeight + 6.0f;
				CameraHeight = FMath::Clamp<float>(CameraHeight, 0, 200);
				CameraBoom->SocketOffset.Set(0.0f, 0.0f, CameraHeight);
				//--------------------------
				CameraLong = CameraLong - 6.0f;
				CameraLong = FMath::Clamp<float>(CameraLong , -200, 450);
				CameraBoom->SocketOffset.Set(0.0f, CameraLong, 0.0f);
			}
			else if (Value > 0) {
				CameraHeight= CameraHeight + 6.0f;
				CameraHeight = FMath::Clamp<float>(CameraHeight, 0, 200);
				CameraBoom->SocketOffset.Set(0.0f, 0.0f, CameraHeight);
				//--------------------------
				CameraLong = CameraLong + 6.0f;
				CameraLong = FMath::Clamp<float>(CameraLong , -200, 450);
				CameraBoom->SocketOffset.Set(0.0f, CameraLong, 0.0f);

			}
			AddMovementInput(Direction, -Value);

		}
		
	}
}

void AOyunDenemeCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		if (!Climbing)
		AddMovementInput(Direction, Value);
	}
}
