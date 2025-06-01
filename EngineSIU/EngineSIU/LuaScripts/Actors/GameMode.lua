
setmetatable(_ENV, { __index = EngineTypes })

-- Template은 AActor라는 가정 하에 작동.

local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.
local FRotator = EngineTypes.FRotator

local SpawnRate = 4.0                  -- 초 단위, 몬스터 생성 주기
local ElapsedTimeSinceLastSpawn = 0.0   -- 누적 시간 트래킹

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name) -- Table에 등록해 준 Name 출력.

end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    -- local this = self.this
    ElapsedTimeSinceLastSpawn = ElapsedTimeSinceLastSpawn + DeltaTime
    if ElapsedTimeSinceLastSpawn >= SpawnRate then
        self:SpawnMonster(DeltaTime)
        ElapsedTimeSinceLastSpawn = 0.0
    end
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

function ReturnTable:SpawnMonster(DeltaTime)
    print("GameMode Spawn Monster Tick: ", DeltaTime)
    -- 예: 로컬 좌표를 랜덤으로 생성해서 몬스터 스폰 테스트

    local randomX = math.random(-140, 140)
    local randomY = math.random(-140, 140)
    local randomZ = 50
    local spawnPos = FVector(randomX, randomY, randomZ)
    
    local spawnRot = FRotator(0, 0, 0)
    print("Spawn Pos : ", spawnPos.X, " " , spawnPos.Y, " " , spawnPos.Z)
    print("Spawn Rot : ", spawnRot.Pitch, " " , spawnRot.Yaw, " " , spawnRot.Roll)
    print("this: ", self.this)

    self.this:SpawnMonster(spawnPos, spawnRot)

    -- self.SpawnMonster()
end

return ReturnTable
