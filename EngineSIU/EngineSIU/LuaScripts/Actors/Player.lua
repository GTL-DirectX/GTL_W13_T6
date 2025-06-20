
setmetatable(_ENV, { __index = EngineTypes })

-- Template은 AActor라는 가정 하에 작동.

local ReturnTable = {
    SmoothedSpeed = 0,
} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.
-- local FRotator = EngineTypes.FRotator
-- 
-- local SpawnRate = 4.0                  -- 초 단위, 몬스터 생성 주기
-- local ElapsedTimeSinceLastSpawn = 0.0   -- 누적 시간 트래킹

local function clamp(val, lo, hi)
    if val < lo then return lo end
    if val > hi then return hi end
    return val
end

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    --     print("BeginPlay ", self.Name) -- Table에 등록해 준 Name 출력.
    local this = self.this

    this.Acceleration = 100000
    this.MaxSpeed = 100000
    this.RawSpeed = 150
    this.PitchSpeed = 100
    this.MaxStunGauge = 40
    this.KnockBackPower = 2500
    this.KnockBackExp = 1
    
    self.CurrentTime = 0

end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    local this = self.this
    
    local moveSpeed = this.MoveSpeed or 0

    if this.State < 3 then
        if this.LinearSpeed <= 1 then
            this.State = 0
        elseif this.LinearSpeed >= 20 then
            this.State = 1
        else
            this.State = 2
        end
    end
    -- (필요하다면) 이후 애니메이션 블렌딩 로직 등...
    -- 예) 이 시점에서 this.State 값에 따라 애니메이션 트랜지션을 처리

    -- 3) 최종 Velocity 보정 (예시: 감속 개념)
    -- this.Velocity = this.Velocity * 0.05
    -- if this.State == 0 and this.MoveSpeed > 100 then
    --     this.State = 1
    -- elseif this.State == 1 and this.MoveSpeed < 90 then
    --     this.State = 0
    -- end
    
    --print(this.State, this.MoveSpeed)
    
    this.Velocity = this.Velocity * 0.01
    
    if (this.ActorLocation.Z < -50 or this.ActorLocation:Length() > 600) then
        self:OnDead()
    end
    
    self.CurrentTime = (self.CurrentTime or 0) + DeltaTime
    
    if self.AttackCoroutine and coroutine.status(self.AttackCoroutine) == "suspended" then
        coroutine.resume(self.AttackCoroutine)
    elseif self.AttackCoroutine and coroutine.status(self.AttackCoroutine) == "dead" then
        self.AttackCoroutine = nil
    end
    
    if self.StunCoroutine and coroutine.status(self.StunCoroutine) == "suspended" then
        coroutine.resume(self.StunCoroutine)
    elseif self.StunCoroutine and coroutine.status(self.StunCoroutine) == "dead" then
        self.StunCoroutine = nil
    end
    
    if self.KnockBackCoroutine and coroutine.status(self.KnockBackCoroutine) == "suspended" then
        coroutine.resume(self.KnockBackCoroutine)
    elseif self.KnockBackCoroutine and coroutine.status(self.KnockBackCoroutine) == "dead" then
        self.KnockBackCoroutine = nil
    end
    
--     ElapsedTimeSinceLastSpawn = ElapsedTimeSinceLastSpawn + DeltaTime
--     if ElapsedTimeSinceLastSpawn >= SpawnRate then
--         self:SpawnMonster(DeltaTime)
--         ElapsedTimeSinceLastSpawn = 0.0
    
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

function ReturnTable:OnDamaged(KnockBackDir)
    this = self.this
    if this.State >= 5 then return end
    
    this.StunGauge = this.StunGauge + 10
    this.KnockBackExp = this.KnockBackExp * 1.2
    this.State = 5

    print(this.KnockBackExp, this.KnockBackPower)
    
    print("OnDamaged 실행")
    print("누적 Damage: ", this.StunGauge)
    
    self:KnockBack(KnockBackDir)
end

function ReturnTable:Stun()
    local this = self.this
    if this.State >= 4 then return end
    
    print("Stun 시작")
    
    this.State = 4
    self.StunCoroutine = coroutine.create(function()
        self:Wait(5.0)
        print("Stun 끝")
    
        this.StunGauge = 0;
        this.State = 0
    end)
    
end

function ReturnTable:KnockBack(KnockBackDir)
    local this = self.this -- local 추가

    print(this.KnockBackPower, this.KnockBackExp)

    this.Velocity = FVector(15000 + KnockBackDir.X * this.KnockBackPower * this.KnockBackExp, 15000 + KnockBackDir.Y * this.KnockBackPower * this.KnockBackExp,
        68000 + this.KnockBackPower * this.KnockBackExp)
        -- fixed : KnockBackDir.Z into 0.01

    self.KnockBackCoroutine = coroutine.create(function()
        -- 넉백 시작

        this:SetControllerVibration(1.0, 1.0)
        -- 1초 대기
        self:Wait(1.0)

        -- 넉백 종료 (코루틴 안에서 처리)
        this:SetControllerVibration(0.0, 0.0)

        this.MoveSpeed = 0

        -- 스턴 체크도 코루틴 안에서
        if this.StunGauge >= this.MaxStunGauge then
            this.State = 0
            self:Stun()
        else
            this.State = 0
        end
    end)
end

function ReturnTable:OnDead()
    local this = self.this
    
    if (this.State >= 6) then return end
        
    print("Dead")
    this:ChangeViewTarget(1)
    this.State = 6
end

function ReturnTable:Wait(duration)
    local startTime = self.CurrentTime or 0
    while (self.CurrentTime or 0) - startTime < duration do
        coroutine.yield()
    end
end


return ReturnTable
