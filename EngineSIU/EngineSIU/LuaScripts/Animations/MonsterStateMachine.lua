AnimFSM = {
    current = "Idle",
    BlendTime = 0.5,
    land = false,
    Update = function(self, dt)
        -- self.current = "Contents/Fbx/Capoeira.fbx"

        -- self.current = "Contents/Bowser/Armature.003|Bowser_Landing"     
        --self.current = "Contents/Bowser/Armature.003|Bowser_Spin"
        -- self.current = "Contents/Bowser/Armature.002|Bowser_Hit"
        self.current = "Contents/Bowser/Armature|Armature.002|Armature.002|Bowser_Hit"
        -- self.current = "Contents/Bowser/Armature|Armature.002|Armature.002|Bowser_Hit"
        self.current = "Contents/Bowser/Armature|Bowser_Land"
        self.current = "Contents/Bowser/Armature|Bowser_Hit3"
        self.current = "Contents/Bowser/Armature|Bowser_Backhit" -- die
        self.current = "Contents/Bowser/Armature|Bowser_Falling"
        self.current = "Contents/Bowser/Armature|Bowser_Spin"
        -- self.current = "Contents/Bowser/Armature.002|Bowser_Die"

        -- self.OwnerCharacter.IsFalling()

        -- and self.land == false
        if (self.OwnerCharacter.IsFallingToDeath) then
            -- print("Anim : Roaring")
            self.current = "Contents/Bowser/Armature|Bowser_Backhit"
            self.BlendTime = 0.0
        elseif (self.OwnerCharacter.IsRoaring) then
            -- print("Anim : Roaring")
            self.current = "Contents/Bowser/Armature|Bowser_Roar"
            self.BlendTime = 0.0
        elseif (self.OwnerCharacter.IsLanding) then
            -- print("Anim : Landing")
            -- print("OwnerCharacter: ", self.OwnerCharacter)
            self.current = "Contents/Bowser/Armature|Bowser_Land"
            self.BlendTime = 1.5
            self.land = true
        -- elseif (self.OwnerCharacter.IsRoaring) then
        --     -- print("Is Roaring")
        --     self.current = "Contents/Bowser/Armature|Bowser_Roar"
        --     self.BlendTime = 0.5
        elseif (self.OwnerCharacter.IsChasing) then
            -- print("Anim : Chasing")
            self.current = "Contents/Bowser/Armature|Bowser_Spin"
            self.BlendTime = 0.5
        elseif (self.OwnerCharacter.IsFalling) then
            -- print("Anim : Falling")
            self.current = "Contents/Bowser/Armature|Bowser_Falling"
            self.BlendTime = 0.5
        else
            print("Roaring / Landing / Chasing / Falling : ", self.OwnerCharacter.IsRoaring, self.OwnerCharacter.IsLanding, self.OwnerCharacter.IsChasing, self.OwnerCharacter.IsFalling)
        end
        if self == nil then
            print("Self is nill!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        end
            -- print(self.OwnerCharacter:GetIsLanding())
        --print("Current Anim : ", self.current)
        
        return {
            anim = self.current,
            blend = self.BlendTime
        }
    end
}

return AnimFSM
