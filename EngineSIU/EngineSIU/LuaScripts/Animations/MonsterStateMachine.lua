AnimFSM = {
    current = "Idle",
    BlendTime = 0.5,
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

        -- print("Anim Test before Falling : ", self.OwnerCharacter)
        if (self.OwnerCharacter.IsFalling) then
            self.current = "Contents/Bowser/Armature|Bowser_Falling"
            self.BlendTime = 0.5
        elseif (self.OwnerCharacter.IsChasing) then
            self.current = "Contents/Bowser/Armature|Bowser_Spin"
            self.BlendTime = 0.5
        else -- Landing은 Notify로 처리 필요
            self.current = "Contents/Bowser/Armature|Bowser_Land"
            self.BlendTime = 0.5
        end
    

        

        return {
            anim = self.current,
            blend = self.BlendTime
        }
    end
}

return AnimFSM