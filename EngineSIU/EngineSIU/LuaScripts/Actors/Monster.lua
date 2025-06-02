
setmetatable(_ENV, { __index = EngineTypes })

-- Template은 AActor라는 가정 하에 작동.

local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.
local FRotator = EngineTypes.FRotator
local Gravity = FVector(0, 0, -9.8)
local GroundZ = 0.0
local Threshold = 0.1


-- 추격 관련 매개 변수
local chaseSpeed = 40.0             -- 유닛당 초속도
local followDuration = 3.0           -- 타겟 갱신 주기
local targetUpdateDistance = 10.0  -- 거리 기준. 이 값 이내면 타겟 갱신





-- ======================
-- 유틸리티 함수
-- ======================

local function IsFalling(currentLocation)
    return (currentLocation.Z - GroundZ) > Threshold
end

local function IsNearGround(currentLocation)
    return math.abs(currentLocation.Z - GroundZ) <= Threshold
end

local function ApplyGravity(currentLocation, DeltaTime)
    return currentLocation + Gravity * DeltaTime
end

local function MoveTowards(currentPos, targetPos, DeltaTime, speed)
    local direction = targetPos - currentPos
    local distance = direction:Length()
    if distance < 1e-3 then return currentPos end
    local step = math.min(speed * DeltaTime, distance)

    local normalizedDir = FVector(direction.X, direction.Y, direction.Z)
    normalizedDir:Normalize()
    print("Dist : ", normalizedDir.X, " " , normalizedDir.Y, normalizedDir.Z)
    
    -- print("Move : " , currentPos + direction * step)
    return currentPos + normalizedDir * step
end

-- ======================
-- 동작 처리 함수
-- ======================

local function HandleFalling(self, DeltaTime)
    local this = self.this
    this.ActorLocation = ApplyGravity(this.ActorLocation, DeltaTime)
    
    -- print("Falling... Z = ", newLocation.Z)
end

local function HandleLanding(self)
    local this = self.this
    local loc = this.ActorLocation
    if not IsNearGround(loc) then
        loc.Z = GroundZ
        this.ActorLocation = loc
    end
    -- print("Landed at Z = ", this.ActorLocation.Z)
end

local function HandleChasing(self, DeltaTime)
    local this = self.this
    this.FollowTimer = this.FollowTimer + DeltaTime

    -- print("FollowTimer: ", this.FollowTimer)
    local curPos = this.ActorLocation
    local targetPos = this.TargetPosition
    local distanceToTarget = (targetPos - curPos):Length()

    if this.FollowTimer >= followDuration or distanceToTarget < targetUpdateDistance then
        this.FollowTimer = 0.0
        this:UpdateTargetPosition()
        targetPos = this:GetTargetPosition()
    end
        -- print("New Target Acquired: ", targetPos.X, targetPos.Y, targetPos.Z)

    -- print("New Target Acquired: ", this.TargetPosition.X, this.TargetPosition.Y, this.TargetPosition.Z)
    this.ActorLocation = MoveTowards(this.ActorLocation, this.TargetPosition, DeltaTime, chaseSpeed)
end


-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name) -- Table에 등록해 준 Name 출력.

end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    local this = self.this
    local currentLocation = this.ActorLocation

    if not this.IsChasing then
        if IsFalling(currentLocation) then
            HandleFalling(self, DeltaTime)
        else
            print("Height  chasing ", currentLocation.Z, this.IsChasing)
            HandleLanding(self)
            this.IsChasing = true
            this:UpdateTargetPosition()
            -- targetPosition = self.this:GetTargetPosition()
        end
    else
        HandleChasing(self, DeltaTime)
    end

    

    -- print("Monster Tick  ", DeltaTime)
    -- 기본적으로 Table로 등록된 변수는 self, Class usertype으로 선언된 변수는 self.this로 불러오도록 설정됨.
    -- sol::property로 등록된 변수는 변수 사용으로 getter, setter 등록이 되어 .(dot) 으로 접근가능하고
    -- 바로 등록된 경우에는 PropertyName() 과 같이 함수 형태로 호출되어야 함.
    -- this.ActorLocation = this.ActorLocation + FVector(1.0, 0.0, 0.0) * DeltaTime -- X 방향으로 이동하도록 선언.

end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason) -- EndPlayReason Type 등록된 이후 사용 가능.
    print("EndPlay")

end

function ReturnTable:Attack(AttackDamage)
    self.GetDamate(AttackDamage)

end

return ReturnTable
