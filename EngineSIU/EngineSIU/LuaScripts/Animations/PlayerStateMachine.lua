AnimFSM = {
    current = "Idle",
    Update = function(self, dt)
        if (self.OwnerCharacter.IsAttacking) then
            self.current = "Contents/Player_3TTook/Armature|Left_Hook"
        else
            self.current = "Contents/Player_3TTook/Armature|Player_Running"
        end
        
        return {
            anim = self.current,
            blend = 0.5
        }
    end
}

return AnimFSM
