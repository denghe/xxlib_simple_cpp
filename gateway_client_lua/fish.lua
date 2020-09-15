return function( this )
    this:SetFunc(function()
        this:SetN(this:GetN() + 2)
    end)
end
