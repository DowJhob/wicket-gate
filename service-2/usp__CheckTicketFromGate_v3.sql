USE [TCDB_Check]
GO
/****** Object:  StoredProcedure [dbo].[usp__CheckTicketFromGate_v3]    Script Date: 16.05.2022 14:44:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<eulle@ya.ru>
-- Create date: <11.02.2022>
-- Description:	<for new FSM>
-- =============================================
ALTER PROCEDURE [dbo].[usp__CheckTicketFromGate_v3]
(
@gateMAC nvarchar(17),
@direction int,
@barcode nvarchar(max),

--@type int output,				-- 1 - ticket
								-- 2 - ticket controller
								-- 3 - covid controller
@check_result int output
)

AS
BEGIN
--DECLARE @result nvarchar(50)
--DECLARE @desc nvarchar(50)

set @check_result = 0x555
	begin try
		DECLARE @userID UNIQUEIDENTIFIER	set @userID		= NULL
		DECLARE @userActive bit	set @userActive		= 'False'
		DECLARE @dateTime datetimeoffset(7)	set @dateTime		= SYSDATETIMEOFFSET ()
		declare @state int set @state = 0

		--set @type = 0

		-- Проверяем турникет по таблице
		--declare @gateId UNIQUEIDENTIFIER
		--select @gateId = dbo.Gate2.IdGate from dbo.Gate2 where GateMAC=@gateMAC --and (CodeGateState=10 or CodeGateState=20)
		--if @gateId is null
		--	begin
		--		set @state = 106 --set @desc = N'Турникет не зарегистрирован!'
		--		set @check_result = 0x0200
		--	end
		--ELSE
			BEGIN
				DECLARE @CodeTicketState INT
				DECLARE @IdTicket UNIQUEIDENTIFIER
				DECLARE @IdEvent UNIQUEIDENTIFIER

				-- Для контроля кол-ва проходов
				DECLARE @passCount INT
				DECLARE @maxPass INT

					--- Тут нормальная проверка
				SELECT
					  @CodeTicketState = ct.CodeTicketState
					, @IdTicket = ct.IdTicket
					, @IdEvent = ct.IdEvent
					, @maxPass = ct.MaxPass
					, @passCount = ct.PassCount
						FROM dbo.CheckTicket ct
					JOIN dbo.CheckEvent ce ON ce.IdEvent = ct.IdEvent
										--AND ce.EntryStartTime < @dateTime
										--AND ce.EntryStopTime > @dateTime
						WHERE ct.TicketNumber = @barcode
										and @dateTime between ce.EntryStartTime and ce.EntryStopTime
										AND ce.AllowEntry = 1
				IF(@IdTicket is not NULL)	-- если есть такой билет
					BEGIN
						--set @type = 0	    -- 1 - ticket
						IF @direction = 1 --'In'
							BEGIN
								--{
								IF (@passCount < @maxPass)
									BEGIN
										IF ((@CodeTicketState = 0) -- если не проходили
											OR (@CodeTicketState = 20) -- если вышли
											OR ((@CodeTicketState = 10) AND (@passCount < @maxPass)) -- если вошли, и еще можно по счетчику
											)
											BEGIN
												set @state = 100   -- set @desc = N'Вход разрешен.'
												set @check_result = 0x0000
											END
										ELSE IF ((@CodeTicketState = 10) AND (@passCount = @maxPass))
											BEGIN
												set @state = 101  --set @desc = N'Вход запрещен! Билет уже на мероприятии!'
												set @check_result = 0x0001
											END
											ELSE
												BEGIN
													set @state = 105 --set @desc = N'Вход запрещен! Неизвестное состояние билета'
													set @check_result = 0x0002
												END
									END
									ELSE
										BEGIN
												set @state = 101  --set @desc = N'Вход запрещен! Билет уже на мероприятии!'
												set @check_result = 0x0001
										END
								--}
							END
						ELSE IF @direction = -1 --'Out'
							BEGIN
								--{
								IF(@passCount > 0)
								BEGIN
									BEGIN
										IF (
											@CodeTicketState = 10 --если вошли
											OR ((@CodeTicketState = 20) and (@passCount > 0)) --если выходили, но еще есть счетчик
											)
											BEGIN
												set @state = 100 --set @desc = N'Выход разрешен.'
												set @check_result = 0x0100
											END
										ELSE IF (@CodeTicketState = 0)
											BEGIN
												set @state = 102 --set @desc = N'Выход запрещен! Билет еще не прошел!'
												set @check_result = 0x0110
											END
										ELSE IF (@CodeTicketState = 20)
											BEGIN
												set @state = 103 --set @desc = N'Выход запрещен! Билет уже покинул мероприятие!'
												set @check_result = 0x0120
											END
									END
								END
								ELSE
								BEGIN
									BEGIN
										set @state = 103 --set @desc = N'Выход запрещен! Билет уже покинул мероприятие!'
										set @check_result = 0x0120
									END
								END
								--}
							END
						ELSE
							BEGIN
								set @state = 105 --set @desc = N'Выход запрещен! Неизвестное состояние билета!'
								set @check_result = 0x0130
							END
					END
				ELSE						-- если нет, поищем бабку контролера
					BEGIN
						select @userID = cht.CheckUser.ID, @userActive = cht.CheckUser.active from cht.CheckUser where cast([key] as varchar(36)) = @barcode
						if @userID is not null and @userActive = 'True'
							BEGIN
						        --set @type = 2	    -- 2 - ticket controller
								set @state = 100 --set @desc = 'Сервисный проход'
								set @check_result = 0x1000
								--Return 10
							END
						ELSE
							BEGIN						-- если нет, поищем ковид полотера
								select @userID = Id, @userActive = active from dbo.checkCovidControler where cast([Key] as varchar(36)) = @barcode
								if @userID is not null and @userActive = 'True'
									BEGIN
										--set @type = 3	    -- 3 - covid controller
										set @state = 100 --set @desc = 'Сервисный проход'
										set @check_result = 0x2000
										--Return 10
									END
								ELSE
									BEGIN
										set @state = 104 --set @desc = N'Билет не зарегистрирован на мероприятии!'
										set @check_result = 0x0300
									END
							END
					END
			END

			insert into dbo.Log2( gateId, userID, spectatorKey, logTime, direction, state2 )
				values( --@gateId
					null, @userID, @barcode, @dateTime, @direction, @state )
	end try

	begin catch

--	SELECT  
	--	ERROR_NUMBER() AS ErrorNumber  
	--	,ERROR_SEVERITY() AS ErrorSeverity  
--		,ERROR_STATE() AS ErrorState  
--		,ERROR_PROCEDURE() AS ErrorProcedure  
--		,ERROR_LINE() AS ErrorLine  
--		,ERROR_MESSAGE() AS ErrorMessage;  
	end catch

END
