AnimFSM = {
    current = "Idle",
    Update = function(self, dt)
        if (self.OwnerCharacter.IsAttacking) then
            print("Attacking 실행 중")
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Left_Hook"
        else
            print("Running 실행 중")
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Player_Running"
        end
        
        return {
            anim = self.current,
            blend = 0.5
        }
    end
}

return AnimFSM
