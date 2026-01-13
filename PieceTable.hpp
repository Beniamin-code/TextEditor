#pragma once

#include<vector>
#include<string>
#include<deque>

enum class BufferType {
	Original,
	Add
};

struct Piece {
	BufferType Source;
	size_t Start;
	size_t Length;
	Piece *Prev, *Next;
};

struct ListOfPieces {
	Piece *First;
	Piece *Last;
	size_t Length;
};

enum class ActionType {
	Insert,
	Delete,
	Replace
};

struct EditAction {
	ActionType Type;
	size_t Index;
	std::string Text;
	std::string ExtraText;
};

class PieceTable {
private:
	std::string m_OriginalBuffer;
	std::string m_AddBuffer;
	ListOfPieces m_Pieces;

	std::vector<size_t>m_LineOffsets;

	std::deque<EditAction> m_UndoDeque;
	std::deque<EditAction> m_RedoDeque;
	const size_t m_Max_History = 1000;

	bool m_IsUndoRedo = false;

	void PushUndo(const EditAction &Action);
	void UpdateLineOffsets_Insert(size_t Index, const std::string &Text);
	void UpdateLineOffsets_Delete(size_t Index, size_t Length);
	bool IsMatchAt(const Piece *p, size_t OffsetInNode, const std::string &Query) const;
public:

	PieceTable(const std::string &InitialText);
	~PieceTable();

	std::string GetTextRange(size_t Start, size_t Length) const;
	std::string GetLine(size_t LineNumber) const;
	std::string GetText() const;
	std::size_t GetLineCount() const;
	std::size_t GetCharCount() const;

	size_t GetLineStartIndex(size_t LineNumber) const;
	size_t GetLineNumberAtIndex(size_t Index) const;


	void Insert(size_t Index, const std::string &TextToInsert);
	void Delete(size_t Index, size_t LengthToDelete);

	size_t Find(const std::string &Query, size_t StartIndex = 0) const;
	std::vector<size_t> FindAll(const std::string &Query) const;
	bool Replace(const std::string &Query, const std::string &Replacement, size_t StartIndex = 0);
	int ReplaceAll(const std::string &Query, const std::string &Replacement);

	void Copy(size_t Index, size_t Length);
	void Cut(size_t Index, size_t Length);
	void Paste(size_t Index);

	void Undo();
	void Redo();

	void ClearList();
};