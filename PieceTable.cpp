#include "PieceTable.hpp"
#include "Clipboard.hpp"
#include <algorithm>

PieceTable::PieceTable(const std::string& InitialText) {
	m_OriginalBuffer = InitialText;
	m_AddBuffer = "";
	m_IsUndoRedo = false;

	m_Pieces.First = nullptr;
	m_Pieces.Last = nullptr;
	m_Pieces.Length = InitialText.length();

	if(m_Pieces.First == NULL) {
		Piece* PieceToAdd = new Piece();

		PieceToAdd->Source = BufferType::Original;
		PieceToAdd->Start = 0;
		PieceToAdd->Length = InitialText.length();
		PieceToAdd->Prev = nullptr;
		PieceToAdd->Next = nullptr;

		m_Pieces.First = PieceToAdd;
		m_Pieces.Last = PieceToAdd;
	}

	m_LineOffsets.push_back(0);
	for(size_t i = 0; i < InitialText.length(); ++i) {
		if(InitialText[i] == '\n') {
			m_LineOffsets.push_back(i + 1);
		}
	}
}

PieceTable::~PieceTable() {
	ClearList();
}

std::string PieceTable::GetText() const{
	std::string FullText = "";
	Piece *p = m_Pieces.First;
	while(p != NULL) {
		if(p->Source == BufferType::Original)
			FullText += m_OriginalBuffer.substr(p->Start, p->Length);
		else
			FullText += m_AddBuffer.substr(p->Start, p->Length);

		p = p->Next;
	}
	

	return FullText;
}

std::string PieceTable::GetTextRange(size_t Start, size_t Length) const {
	if(Length == 0 || m_Pieces.Length == 0 || Start >= m_Pieces.Length) return "";

	if(Start + Length > m_Pieces.Length) {
		Length = m_Pieces.Length - Start;
	}

	std::string Result;
	Result.reserve(Length); 

	Piece *Current = m_Pieces.First;
	size_t CurrentGlobalIndex = 0;

	while(Current != nullptr) {
		if(Start < CurrentGlobalIndex + Current->Length) {
			break; 
		}
		CurrentGlobalIndex += Current->Length;
		Current = Current->Next;
	}

	size_t CopiedSoFar = 0;
	size_t OffsetInNode = Start - CurrentGlobalIndex;

	while(CopiedSoFar < Length && Current != nullptr) {
		size_t AvailableInNode = Current->Length - OffsetInNode;
		size_t ToCopy = std::min(AvailableInNode, Length - CopiedSoFar);

		const std::string &SourceBuffer = (Current->Source == BufferType::Original)
			? m_OriginalBuffer
			: m_AddBuffer;

		Result += SourceBuffer.substr(Current->Start + OffsetInNode, ToCopy);
		CopiedSoFar += ToCopy;
		Current = Current->Next;
		OffsetInNode = 0;
	}

	return Result;
}

std::string PieceTable::GetLine(size_t LineNumber) const {
	if(LineNumber >= m_LineOffsets.size()) {
		return "";
	}

	size_t StartIndex = m_LineOffsets[LineNumber];
	size_t Length = 0;

	if(LineNumber + 1 < m_LineOffsets.size()) {
		size_t NextLineStart = m_LineOffsets[LineNumber + 1];
		Length = (NextLineStart - 1) - StartIndex;
	}
	else {
		Length = m_Pieces.Length - StartIndex;
	}

	if(Length == 0) return "";

	return GetTextRange(StartIndex, Length);
}

size_t PieceTable::GetLineCount() const {
	return m_LineOffsets.size();
}

size_t PieceTable::GetCharCount() const {
	return m_Pieces.Length;
}

void PieceTable::ClearList() {
	Piece *Current = m_Pieces.First;

	while(Current != nullptr) {
		Piece *Next = Current->Next;
		delete Current;
		Current = Next;
	}

	m_Pieces.First = nullptr;
	m_Pieces.Last = nullptr;
	m_Pieces.Length = 0;
}

void PieceTable::PushUndo(const EditAction &Action) {
	if(m_UndoDeque.size() > m_Max_History) {
		m_UndoDeque.pop_front();
	}

	m_UndoDeque.push_back(Action);
}

void PieceTable::Undo() {
	if(m_UndoDeque.empty()) return;

	EditAction LastAction = m_UndoDeque.back();
	m_UndoDeque.pop_back();

	m_IsUndoRedo = true;

	if(LastAction.Type == ActionType::Insert) {
		Delete(LastAction.Index, LastAction.Text.length());
	} 
	else if(LastAction.Type == ActionType::Delete) {
		Insert(LastAction.Index, LastAction.Text);
	}
	else if(LastAction.Type == ActionType::Replace) {
		Delete(LastAction.Index, LastAction.Text.length());
		Insert(LastAction.Index, LastAction.ExtraText);
	}

	m_IsUndoRedo = false;
	m_RedoDeque.push_back(LastAction);
}

void PieceTable::Redo() {
	if(m_RedoDeque.empty()) return;

	EditAction NextAction = m_RedoDeque.back();
	m_RedoDeque.pop_back();

	m_IsUndoRedo = true;

	if(NextAction.Type == ActionType::Insert) {
		Insert(NextAction.Index, NextAction.Text);
	}
	else if(NextAction.Type == ActionType::Delete) {
		Delete(NextAction.Index, NextAction.Text.length());
	}
	else if(NextAction.Type == ActionType::Replace) {
		Delete(NextAction.Index, NextAction.Text.length());
		Insert(NextAction.Index, NextAction.Text);
	}

	m_IsUndoRedo = false;
	PushUndo(NextAction);
}

void PieceTable::Insert(size_t SelectedIndex, const std::string &TextToInsert) {
	if(TextToInsert.empty()) return;

	if(!m_IsUndoRedo) {
		EditAction Action;
		Action.Type = ActionType::Insert;
		Action.Index = SelectedIndex;
		Action.Text = TextToInsert;

		PushUndo(Action);

		m_RedoDeque.clear();
	}

	UpdateLineOffsets_Insert(SelectedIndex, TextToInsert);

	size_t StartOffset = m_AddBuffer.length();
	m_AddBuffer += TextToInsert;

	Piece *NewPiece = new Piece();
	NewPiece->Source = BufferType::Add;
	NewPiece->Start = StartOffset;
	NewPiece->Length = TextToInsert.length();
	NewPiece->Next = nullptr;
	NewPiece->Prev = nullptr;


	if(m_Pieces.First == nullptr) {
		m_Pieces.First = NewPiece;
		m_Pieces.Last = NewPiece;
		m_Pieces.Length = TextToInsert.length(); 
		return;
	}

	Piece *Target = m_Pieces.First;
	size_t CurrentGlobalIndex = 0;

	while(Target != nullptr) {
		if(SelectedIndex <= CurrentGlobalIndex + Target->Length) break; 
		CurrentGlobalIndex += Target->Length;
		Target = Target->Next;
	}

	if(Target == nullptr) {
		Piece *Last = m_Pieces.Last;
		Last->Next = NewPiece;
		NewPiece->Prev = Last;
		m_Pieces.Last = NewPiece;
		m_Pieces.Length += TextToInsert.length();
		return;
	}

	size_t LocalOffset = SelectedIndex - CurrentGlobalIndex;

	if(LocalOffset == 0) {
		NewPiece->Prev = Target->Prev;
		NewPiece->Next = Target;

		if(Target->Prev) Target->Prev->Next = NewPiece;
		else m_Pieces.First = NewPiece; 
		Target->Prev = NewPiece;
	}
	
	else if(LocalOffset == Target->Length) {
		NewPiece->Next = Target->Next;
		NewPiece->Prev = Target;

		if(Target->Next)Target->Next->Prev = NewPiece; 
		else m_Pieces.Last = NewPiece; 
		Target->Next = NewPiece;
	}
	
	else {
		
		Piece *RightPiece = new Piece();
		RightPiece->Source = Target->Source;
		RightPiece->Start = Target->Start + LocalOffset; 
		RightPiece->Length = Target->Length - LocalOffset; 

		Target->Length = LocalOffset; 

		Piece *OldNext = Target->Next;

		Target->Next = NewPiece;
		NewPiece->Prev = Target;

		NewPiece->Next = RightPiece;
		RightPiece->Prev = NewPiece;

		RightPiece->Next = OldNext;
		if(OldNext) OldNext->Prev = RightPiece;
		else m_Pieces.Last = RightPiece;
	}

	m_Pieces.Length += TextToInsert.length();
}

void PieceTable::Delete(size_t StartIndex, size_t LengthToDelete) {
	if(LengthToDelete == 0 || m_Pieces.Length == 0) return;

	if(!m_IsUndoRedo) {
		std::string deletedText = GetTextRange(StartIndex, LengthToDelete);

		EditAction Action;
		Action.Type = ActionType::Delete;
		Action.Index = StartIndex;
		Action.Text = deletedText;

		PushUndo(Action);
		m_RedoDeque.clear();
	}

	UpdateLineOffsets_Delete(StartIndex, LengthToDelete);

	Piece *Current = m_Pieces.First;
	size_t CurrentGlobalIndex = 0;

	while(Current != nullptr) {
		if(StartIndex < CurrentGlobalIndex + Current->Length) break;
		CurrentGlobalIndex += Current->Length;
		Current = Current->Next;
	}

	if(!Current) return; 

	size_t OffsetInNode = StartIndex - CurrentGlobalIndex;
	size_t RemainingToDelete = LengthToDelete;

	while(RemainingToDelete > 0 && Current != nullptr) {
		size_t AvailableInThisNode = Current->Length - OffsetInNode;

		if(RemainingToDelete < AvailableInThisNode) {
			
			Piece *RightPiece = new Piece();
			RightPiece->Source = Current->Source;
			RightPiece->Start = Current->Start + OffsetInNode + RemainingToDelete;
			RightPiece->Length = AvailableInThisNode - RemainingToDelete;

			RightPiece->Next = Current->Next;
			RightPiece->Prev = Current;
			if(Current->Next) Current->Next->Prev = RightPiece;
			else m_Pieces.Last = RightPiece;
			Current->Next = RightPiece;

			Current->Length = OffsetInNode;

			RemainingToDelete = 0;
		}

		else {
			size_t deleteFromThis = AvailableInThisNode; 

			if(OffsetInNode == 0 && deleteFromThis == Current->Length) {
				Piece *ToDelete = Current;
				Current = Current->Next; 

				if(ToDelete->Prev) ToDelete->Prev->Next = ToDelete->Next;
				else m_Pieces.First = ToDelete->Next;

				if(ToDelete->Next) ToDelete->Next->Prev = ToDelete->Prev;
				else m_Pieces.Last = ToDelete->Prev;

				delete ToDelete; 
			}

			else if(OffsetInNode > 0) {
				Current->Length = OffsetInNode;
				Current = Current->Next;     
			}
			
			else if(OffsetInNode == 0) {
				Current->Start += deleteFromThis; 
				Current->Length -= deleteFromThis; 
				Current = Current->Next;
			}

			RemainingToDelete -= deleteFromThis;
			OffsetInNode = 0;
		}
	}

	m_Pieces.Length -= LengthToDelete;
}

void PieceTable::UpdateLineOffsets_Insert(size_t Index, const std::string &Text) {
	auto It = std::upper_bound(m_LineOffsets.begin(), m_LineOffsets.end(), Index);

	for(auto ShiftIt = It; ShiftIt != m_LineOffsets.end(); ++ShiftIt) {
		*ShiftIt += Text.length();
	}

	std::vector<size_t> NewOffsets;
	for(size_t i = 0; i < Text.length(); ++i) {
		if(Text[i] == '\n') {
			NewOffsets.push_back(Index + i + 1);
		}
	}

	if(!NewOffsets.empty()) {
		m_LineOffsets.insert(It, NewOffsets.begin(), NewOffsets.end());
	}
}

void PieceTable::UpdateLineOffsets_Delete(size_t Index, size_t Length) {
	size_t EndIndex = Index + Length;

	auto ItStart = std::upper_bound(m_LineOffsets.begin(), m_LineOffsets.end(), Index);
	auto ItEnd = std::upper_bound(m_LineOffsets.begin(), m_LineOffsets.end(), EndIndex);

	ItStart = m_LineOffsets.erase(ItStart, ItEnd);

	for(auto It = ItStart; It != m_LineOffsets.end(); ++It) {
		*It -= Length;
	}
}

bool PieceTable::IsMatchAt(const Piece *p, size_t OffsetInNode, const std::string &Query) const {
	const Piece *CurrentPiece = p;
	size_t CurrentOffset = OffsetInNode;

	for(size_t i = 0; i < Query.length(); ++i) {
		if(CurrentPiece == nullptr) return false;

		char C;
		if(CurrentPiece->Source == BufferType::Original) {
			C = m_OriginalBuffer[CurrentPiece->Start + CurrentOffset];
		} else {
			C = m_AddBuffer[CurrentPiece->Start + CurrentOffset];
		}

		if(C != Query[i]) return false;

		CurrentOffset++;

		if(CurrentOffset == CurrentPiece->Length) {
			CurrentPiece = CurrentPiece->Next;
			CurrentOffset = 0;
		}
	}

	return true;
}

size_t PieceTable::Find(const std::string &Query, size_t StartIndex) const {
	if(Query.empty() || m_Pieces.Length == 0
		|| (StartIndex + Query.length() > m_Pieces.Length)) return std::string::npos;

	Piece *CurrentPiece = m_Pieces.First;
	size_t CurrentGlobalPosition = 0;

	while(CurrentPiece != nullptr) {
		if(CurrentGlobalPosition + CurrentPiece->Length > StartIndex) break;
		CurrentGlobalPosition += CurrentPiece->Length;
		CurrentPiece = CurrentPiece->Next;
	}

	if(CurrentPiece == nullptr) return std::string::npos;

	size_t OffsetInNode = StartIndex - CurrentGlobalPosition;
	size_t MaxSearchIndex = m_Pieces.Length - Query.length();
	size_t SearchPosition = StartIndex;

	while(SearchPosition <= MaxSearchIndex && CurrentPiece != nullptr) {
		if(IsMatchAt(CurrentPiece, OffsetInNode, Query)) return SearchPosition;

		SearchPosition++;
		OffsetInNode++;

		if(OffsetInNode == CurrentPiece->Length) {
			CurrentPiece = CurrentPiece->Next;
			OffsetInNode = 0;
		}

	}

	return std::string::npos;
}

std::vector<size_t> PieceTable::FindAll(const std::string &Query) const {
	std::vector<size_t> Positions;

	if(Query.empty()) return Positions;

	size_t CurrentIndex = 0;

	while(true) {
		size_t FoundAt = Find(Query, CurrentIndex);

		if(FoundAt == std::string::npos) break;

		Positions.push_back(FoundAt);

		CurrentIndex = FoundAt + 1;
	}

	return Positions;
}

bool PieceTable::Replace(const std::string &Query, const std::string &Replacement, size_t StartIndex) {
	size_t FoundIndex = Find(Query, StartIndex);

	if(FoundIndex == std::string::npos) return false;

	if(!m_IsUndoRedo) {
		EditAction Action;
		Action.Type = ActionType::Replace;
		Action.Index = FoundIndex;
		Action.Text = Replacement;
		Action.ExtraText = Query;

		PushUndo(Action);
		m_RedoDeque.clear();
	}

	bool WasUndo = m_IsUndoRedo;
	m_IsUndoRedo = true;

	Delete(FoundIndex, Query.length());
	Insert(FoundIndex, Replacement);

	m_IsUndoRedo = WasUndo;

	return true;
}

int PieceTable::ReplaceAll(const std::string &Query, const std::string &Replacement) {
	int Count = 0;
	size_t CurrentIndex = 0;

	while(true) {
		size_t FoundAt = Find(Query, CurrentIndex);
		if(FoundAt == std::string::npos) break;

		Replace(Query, Replacement, CurrentIndex);
		Count++;

		CurrentIndex = FoundAt + Replacement.length();
	}

	return Count;
}
 
void PieceTable::Copy(size_t Index, size_t Length) {
	std::string TextToCopy = GetTextRange(Index, Length);

	if(!TextToCopy.empty()) {
		Platform::SetClipboardText(TextToCopy);
	}
}

void PieceTable::Cut(size_t Index, size_t Length) {
	std::string TextToCut = GetTextRange(Index, Length);

	if(TextToCut.empty()) return;

	Platform::SetClipboardText(TextToCut);

	Delete(Index, Length);
}

void PieceTable::Paste(size_t Index) {
	std::string TextToPaste = Platform::GetClipboardText();
	
	if(!TextToPaste.empty()) {
		Insert(Index, TextToPaste);
	}
}
