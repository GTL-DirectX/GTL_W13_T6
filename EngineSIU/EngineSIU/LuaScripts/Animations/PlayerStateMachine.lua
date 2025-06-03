AnimFSM = {
    current = "Idle",
    Update = function(self, dt)
        State = self.OwnerCharacter.State
        
        if State == 0 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Player_Running"
        elseif State == 1 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Player_Running"
        elseif State == 2 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Player_Jumping"
        elseif State == 3 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Left_Hook"
        elseif State == 5 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Head_Hit"
        end
        
        return {
            anim = self.current,
            blend = 0.5
        }
    end
}

return AnimFSM
