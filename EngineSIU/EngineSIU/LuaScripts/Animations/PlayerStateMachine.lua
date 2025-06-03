AnimFSM = {
    current = "Idle",
    BlendTime = 0.5,
    Update = function(self, dt)
        State = self.OwnerCharacter.State
        
        if State == 0 then
            self.current = "Contents/Player_3TTook/Armature|Player_Idle"
            self.BlendTime = 0.5
        elseif State == 1 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Player_Running"
            self.BlendTime = 2.0
        elseif State == 7 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Player_Jumping"
            self.BlendTime = 0.0
        elseif State == 3 then
            self.current = "Contents/Player_3TTook/Armature|Armature|Armature|Left_Hook"
            self.BlendTime = 0.0
        elseif State == 2 then
            self.current = "Contents/Player_3TTook/Armature|Player_Walk"
            self.BlendTime = 0.5
        end
        
        return {
            anim = self.current,
            blend = self.BlendTime
        }
    end
}

return AnimFSM
